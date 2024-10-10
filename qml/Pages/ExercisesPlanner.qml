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

	required property QmlItemManager itemManager
	property alias currentPage: splitView.currentItem
	property PageScrollButtons navButtons: null

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
		height: parent.height

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
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
				color: "black"
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

	footer: ToolBar {
		id: splitToolBar
		width: parent.width
		height: footerHeight

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			id: btnClearPlan
			text: qsTr("Clear")
			imageSource: "clear.png"
			textUnderIcon: true
			enabled: currentPage ? currentPage.splitModel.count > 1 : false
			fixedSize: true
			rounded: false
			flat: false
			width: footerHeight
			height: footerHeight
			anchors {
				left: parent.left
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				currentPage.splitModel.clear();
				currentPage.appendNewExerciseToDivision();
				requestSimpleExercisesList(null, false);
			}
		}

		TPButton {
			id: btnSwapPlan
			text: currentPage ? currentPage.splitModel.splitLetter() + " <-> " + currentPage.swappableLetter : "A <-> B"
			imageSource: "swap.png"
			textUnderIcon: true
			visible: currentPage ? currentPage.bCanSwapPlan : false
			fixedSize: true
			rounded: false
			flat: false
			width: footerHeight
			height: footerHeight
			anchors {
				left: btnClearPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: itemManager.swapMesoPlans(currentPage.splitModel.splitLetter(), currentPage.swappableLetter);
		}

		TPButton {
			id: btnImExport
			text: qsTr("In/Ex")
			imageSource: "import-export.png"
			textUnderIcon: true
			fixedSize: true
			rounded: false
			flat: false
			width: footerHeight
			height: footerHeight

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
			width: 70
			height: footerHeight

			anchors {
				right: parent.right
				rightMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: currentPage.appendNewExerciseToDivision();
		} //btnAddExercise
	}

	SimpleExercisesListPanel {
		id: exercisesPane
		parentPage: pagePlanner
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		exercisesPane.itemThatRequestedSimpleList = visible ? object : null;
		exercisesPane.bEnableMultipleSelection = multipleSel;
		exercisesPane.visible = visible;
	}

	function createNavButtons() {
		if (navButtons === null) {
			var component = Qt.createComponent("qrc:/qml/ExercisesAndSets/PageScrollButtons.qml", Qt.Asynchronous);

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

	function setScrollBarPosition(pos) {
		if (currentPage)
			currentPage.setScrollBarPosition(pos);
	}

	function insertSplitPage(page: Item, idx: int) {
		splitView.insertItem(idx, page);
	}

	property TPFloatingMenuBar imExportMenu: null
	readonly property bool bExportEnabled: currentPage ? currentPage.splitModel.count > 1 : false

	onBExportEnabledChanged: {
		if (imExportMenu) {
			imExportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				imExportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	function showInExMenu() {
		if (imExportMenu === null) {
			var imExportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			imExportMenu = imExportMenuComponent.createObject(page, { parentPage: pagePlanner });
			imExportMenu.addEntry(qsTr("Import"), "import.png", 0, true);
			imExportMenu.addEntry(qsTr("Import from ") + currentPage.prevMesoName, "import.png", 0, currentPage.prevMesoId >= 0);
			imExportMenu.addEntry(qsTr("Export"), "save-day.png", 1, true);
			if (Qt.platform.os === "android")
				imExportMenu.addEntry(qsTr("Share"), "export.png", 2, true);
			imExportMenu.menuEntrySelected.connect(selectedMenuOption);
		}
		imExportMenu.setMenuText(1)
		imExportMenu.show(btnImExport, 0);
	}

	function selectedMenuOption(menuid) {
		switch (menuid) {
			case 0: mainwindow.chooseFileToImport(); break;
			case 1: currentPage.showImportFromPreviousMesoMessage(); break;
			case 2: exportTypeTip.init(false); break;
			case 3: exportTypeTip.init(true); break;
		}
	}

	TPBalloonTip {
		id: exportTypeTip
		title: bShare ? qsTr("What do you want to share?") : qsTr("What to you want to export?")
		imageSource: "export"
		button1Text: qsTr("Entire plan")
		button2Text: qsTr("Just this split")
		parentPage: pagePlanner

		onButton1Clicked: itemManager.exportMesoSplit(bShare, "X");
		onButton2Clicked: itemManager.exportMesoSplit(bShare, currentPage.splitModel.splitLetter());

		property bool bShare: false

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}
} //Page
