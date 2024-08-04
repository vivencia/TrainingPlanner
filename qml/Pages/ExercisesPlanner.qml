import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "../"
import "../inexportMethods.js" as INEX
import "../TPWidgets"
import "../ExercisesAndSets"

import com.vivenciasoftware.qmlcomponents

TPPage {
	id: pagePlanner
	objectName: "exercisesPlanner"

	property alias currentPage: splitView.currentItem
	property bool bEnableMultipleSelection: false
	property bool bShowSimpleExercisesList: false
	property Item itemThatRequestedSimpleList: null
	property PageScrollButtons navButtons: null
	property TPFloatingMenuBar imexportMenu: null
	readonly property bool bExportEnabled: splitView.currentIndex >= 0 ? currentPage.splitModel.count > 1 : false

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
		objectName: "splitSwipeView"
		currentIndex: -1
		interactive: !exercisesPane.visible
		height: parent.height
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}
		onCurrentIndexChanged: {
			createNavButtons();
			currentItem.init();
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
			color: AppSettings.fontColor
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
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			id: btnSave
			text: qsTr("Save")
			imageSource: "save-day.png"
			textUnderIcon: true
			rounded: false
			flat: false
			enabled: splitView.currentIndex >= 0 ? currentPage.splitModel.modified : false
			fixedSize: true
			width: footerHeight
			height: footerHeight

			anchors {
				left: parent.left
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				appDB.saveMesoSplitComplete(currentPage.splitModel);
				requestSimpleExercisesList(null, false, false);
			}
		}

		TPButton {
			id: btnClearPlan
			text: qsTr("Clear")
			imageSource: "clear.png"
			textUnderIcon: true
			enabled: splitView.currentIndex >= 0 ? currentPage.splitModel.count > 1 : false
			fixedSize: true
			rounded: false
			flat: false
			width: footerHeight
			height: footerHeight
			anchors {
				left: btnSave.right
				leftMargin: 3
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

			onClicked: appDB.swapMesoPlans(currentPage.splitModel.splitLetter(), currentPage.swappableLetter);
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

			onClicked: INEX.showInExMenu(pagePlanner, true);
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

			onClicked: {
				createNavButtons();
				currentPage.appendNewExerciseToDivision();
			}
		} //btnAddExercise
	}

	SimpleExercisesListPanel {
		id: exercisesPane
		parentPage: pagePlanner
	}

	Component.onCompleted: appDB.getCompleteMesoSplit();

	function requestSimpleExercisesList(object, visible, multipleSel) {
		itemThatRequestedSimpleList = visible ? object : null;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
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

	TPComplexDialog {
		id: exportTypeTip
		customStringProperty1: bShare ? qsTr("What do you want to share?") : qsTr("What to you want to export?")
		customStringProperty2: qsTr("Human readable?")
		customStringProperty3: "export.png"
		button1Text: qsTr("Entire plan")
		button2Text: qsTr("Just this split")
		customItemSource: "TPDialogWithMessageAndCheckBox.qml"
		parentPage: pagePlanner

		onButton1Clicked: appDB.exportMesoSplit("X", bShare, customBoolProperty1);
		onButton2Clicked: appDB.exportMesoSplit(currentPage.splitModel.splitLetter(), bShare, customBoolProperty1);

		property bool bShare: false

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}
} //Page
