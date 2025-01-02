import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "../"
import "../TPWidgets"
import "../ExercisesAndSets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: pagePlanner
	objectName: "exercisesPlanner"

	required property SplitManager splitManager
	property PageScrollButtons navButtons: null
	property MesoSplitPlanner currentSplitPage: null

	signal exerciseSelectedFromSimpleExercisesList();
	signal simpleExercisesListClosed();

	Keys.onPressed: (event) => {
		if (event.key === mainwindow.backKey) {
			event.accepted = true;
			if (splitView.currentIndex === 0)
				mainwindow.popFromStack();
			else
				splitView.decrementCurrentIndex();
		}
	}

	SwipeView {
		id: splitView
		interactive: !exercisesPane.visible
		anchors.fill: parent

		onCurrentIndexChanged: {
			currentSplitPage = splitManager.setCurrentPage(currentIndex);
			currentSplitPage.navButtons = navButtons;
		}
	} //SwipeView

	PageIndicator {
		id: indicator
		count: splitView.count
		currentIndex: splitView.currentIndex
		visible: !exercisesPane.visible
		height: 20

		delegate: Label {
			width: 20
			height: 20
			text: splitView.itemAt(index).splitModel.splitLetter()
			color: appSettings.fontColor
			font.bold: true
			fontSizeMode: Text.Fit
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter

			required property int index

			background: Rectangle {
				radius: width/2
				opacity: index === indicator.currentIndex ? 0.95 : pressed ? 0.7 : 0.45
				color: appSettings.paneBackgroundColor
			}

			Behavior on opacity {
				OpacityAnimator {
					duration: 200
				}
			}
		}

		anchors {
			bottom: parent.bottom
			bottomMargin: 10
			horizontalCenter: parent.horizontalCenter
		}
	}

	footer: TPToolBar {
		id: splitToolBar
		height: footerHeight

		readonly property int buttonWidth: width*0.22

		TPButton {
			id: btnClearPlan
			text: qsTr("Clear")
			imageSource: "clear.png"
			textUnderIcon: true
			enabled: splitManager.hasExercises
			fixedSize: true
			rounded: false
			flat: false
			width: splitToolBar.buttonWidth
			height: footerHeight - 4

			anchors {
				left: parent.left
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: splitManager.currentSplitModel.clearExercises();
		}

		TPButton {
			id: btnSwapPlan
			text: splitManager.currentSplitLetter + " <-> " + splitManager.currentSwappableLetter
			imageSource: "swap.png"
			textUnderIcon: true
			visible: splitManager.canSwapExercises
			fixedSize: true
			rounded: false
			flat: false
			width: splitToolBar.buttonWidth
			height: footerHeight - 4

			anchors {
				left: btnClearPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: splitManager.swapMesoPlans();
		}

		TPButton {
			id: btnImExport
			text: qsTr("In/Ex")
			imageSource: "import-export.png"
			textUnderIcon: true
			fixedSize: true
			rounded: false
			flat: false
			width: splitToolBar.buttonWidth
			height: footerHeight - 4

			anchors {
				left: btnSwapPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: showInExMenu();
		}

		TPButton {
			id: btnAddExercise
			text: qsTr("+ Exercise")
			imageSource: "exercises-add.png"
			textUnderIcon: true
			fixedSize: true
			rounded: false
			flat: false
			width: splitToolBar.buttonWidth*1.3
			height: footerHeight - 4

			anchors {
				right: parent.right
				rightMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: currentSplitPage.appendNewExerciseToDivision();
		} //btnAddExercise
	}

	SimpleExercisesListPanel {
		id: exercisesPane
		parentPage: pagePlanner
		onExerciseSelected: exerciseSelectedFromSimpleExercisesList();
		onListClosed: simpleExercisesListClosed();
	}

	function showSimpleExercisesList(multipleSel: bool): void {
		exercisesPane.bEnableMultipleSelection = multipleSel;
		exercisesPane.open();
	}

	function hideSimpleExercisesList(): void {
		exercisesPane.visible = false;
	}

	function createNavButtons(): void {
		if (navButtons === null) {
			let component = Qt.createComponent("qrc:/qml/ExercisesAndSets/PageScrollButtons.qml", Qt.Asynchronous);

			function finishCreation() {
				navButtons = component.createObject(pagePlanner, { ownerPage: pagePlanner });
				navButtons.scrollTo.connect(setScrollBarPosition);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	function setScrollBarPosition(pos): void {
		if (currentSplitPage)
			currentSplitPage.setScrollBarPosition(pos);
	}

	function insertSplitPage(page: Item, idx: int): void {
		splitView.insertItem(idx, page);
	}

	readonly property bool bExportEnabled: splitManager.hasExercises
	onBExportEnabledChanged: {
		if (imExportMenu) {
			imExportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				imExportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	property TPFloatingMenuBar imExportMenu: null
	function showInExMenu(): void {
		if (imExportMenu === null) {
			let imExportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			imExportMenu = imExportMenuComponent.createObject(pagePlanner, { parentPage: pagePlanner });
			imExportMenu.addEntry(qsTr("Import"), "import.png", 0, true);
			if (splitManager.prevMesoName().length > 0)
				imExportMenu.addEntry(qsTr("Import from ") + splitManager.prevMesoName(), "import.png", 1, true);
			imExportMenu.addEntry(qsTr("Export"), "export.png", 2, true);
			if (Qt.platform.os === "android")
				imExportMenu.addEntry(qsTr("Share"), "export.png", 3, true);
			imExportMenu.menuEntrySelected.connect(selectedMenuOption);
		}
		imExportMenu.show2(btnImExport, 0);
	}

	function selectedMenuOption(menuid): void {
		switch (menuid) {
			case 0: splitManager.importMesoSplit(); break;
			case 1: msgDlgImport.show(-1); break;
			case 2: exportTypeTip.init(false); break;
			case 3: exportTypeTip.init(true); break;
		}
	}

	TPBalloonTip {
		id: msgDlgImport
		title: qsTr("Import Exercises Plan?")
		message: qsTr("Import the exercises plan for training division <b>") + splitManager.currentSplitLetter +
						 qsTr("</b> from <b>") + splitManager.prevMesoName() + "</b>?"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "remove"
		parentPage: pagePlanner

		onButton1Clicked: splitManager.loadSplitFromPreviousMeso();
	} //TPBalloonTip

	TPBalloonTip {
		id: exportTypeTip
		title: bShare ? qsTr("What do you want to share?") : qsTr("What to you want to export?")
		imageSource: "export"
		closeButtonVisible: true
		button1Text: qsTr("Entire plan")
		button2Text: qsTr("Just this split")
		parentPage: pagePlanner

		onButton1Clicked: splitManager.exportMesoSplit(bShare, "X");
		onButton2Clicked: splitManager.exportMesoSplit(bShare, currentPage.splitModel.splitLetter());

		property bool bShare: false

		function init(share: bool): void {
			bShare = share;
			show(-1);
		}
	}

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise?")
		message: exerciseName + qsTr("\nThis action cannot be undone.")
		imageSource: "remove"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		onButton1Clicked: splitManager.currentSplitModel.removeExercise(removeRow);
		parentPage: pagePlanner

		property int removeRow
		property string exerciseName

		function init(row: int, exercise: string): void {
			removeRow = row;
			exerciseName = exercise;
			show(-1);
		}
	} //TPBalloonTip

	function showDeleteDialog(row: int, exercise: string): void {
		msgDlgRemove.init(row, exercise);
	}
} //Page
