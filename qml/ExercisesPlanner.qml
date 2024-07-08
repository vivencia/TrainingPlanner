import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "inexportMethods.js" as INEX
import com.vivenciasoftware.qmlcomponents

Page {
	id: pagePlanner
	objectName: "exercisesPlanner"
	width: windowWidth
	height: windowHeight

	property alias currentPage: splitView.currentItem
	property bool bEnableMultipleSelection: false
	property bool bShowSimpleExercisesList: false
	property var itemThatRequestedSimpleList: null
	property var navButtons: null

	property var imexportMenu: null
	readonly property bool bExportEnabled: splitView.currentIndex >= 0 ? currentPage.splitModel.count > 1 : false

	onBExportEnabledChanged: {
		if (imexportMenu) {
			imexportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				imexportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	Keys.onBackPressed: (event) => {
		event.accepted = true;
		if (exercisesPane.visible)
			requestSimpleExercisesList(null, false, false);
		else
			pagePlanner.StackView.pop();
	}

	SwipeView {
		id: splitView
		objectName: "splitSwipeView"
		currentIndex: -1
		interactive: !exercisesPane.shown
		height: parent.height
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		onCurrentIndexChanged: currentItem.init();
	} //SwipeView

	PageIndicator {
		id: indicator
		count: splitView.count
		currentIndex: splitView.currentIndex
		visible: !exercisesPane.shown
		height: 20
		anchors {
			bottom: parent.bottom
			horizontalCenter: parent.horizontalCenter
		}
	}

	footer: ToolBar {
		id: splitToolBar
		width: parent.width
		height: 55
		visible: !bShowSimpleExercisesList

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
			width: 55
			height: 55

			anchors {
				left: parent.left
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				appDB.updateMesoSplitComplete(currentPage.splitModel);
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
			width: 55
			height: 55
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
			text: splitView.currentIndex >= 0 ? currentPage.splitModel.splitLetter + " <-> " + currentPage.swappableLetter : "A <-> B"
			imageSource: "swap.png"
			textUnderIcon: true
			visible: currentPage ? currentPage.bCanSwapPlan : false
			fixedSize: true
			rounded: false
			flat: false
			width: 55
			height: 55
			anchors {
				left: btnClearPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: appDB.swapMesoPlans(currentPage.splitModel.splitLetter, currentPage.swappableLetter);
		}

		TPButton {
			id: btnImExport
			text: qsTr("In/Ex")
			imageSource: "import-export.png"
			textUnderIcon: true
			fixedSize: true
			rounded: false
			flat: false
			width: 55
			height: 55
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
			height: 55
			anchors {
				right: parent.right
				rightMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				if (navButtons === null)
					createNavButtons();
				currentPage.appendNewExerciseToDivision();
			}
		} //btnAddExercise
	}

	SimpleExercisesListPanel {
		id: exercisesPane
	}

	Component.onCompleted: {
		appDB.getCompleteMesoSplit();
		pagePlanner.StackView.activating.connect(pageActivation);
		pagePlanner.StackView.onDeactivating.connect(pageDeActivation);
	}

	function pageActivation() {
		if (navButtons)
			navButtons.visible = true;
	}

	function pageDeActivation() {
		if (navButtons)
			navButtons.visible = false;
	}

	function createNavButtons() {
		if (navButtons === null) {
			var component = Qt.createComponent("PageScrollButtons.qml", Qt.Asynchronous);

			function finishCreation() {
				navButtons = component.createObject(pagePlanner, {});
				navButtons.scrollTo.connect(currentPage.setScrollBarPosition);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		itemThatRequestedSimpleList = object;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
		if (navButtons !== null) {
			if (visible)
				navButtons.hideButtons();
			else
				navButtons.showButtons();
		}
	}

	function hideSimpleExerciseList() {
		exercisesPane.shown = false;
		if (navButtons !== null)
			navButtons.showButtons();
	}

	function insertSplitPage(page: Item, idx: int) {
		splitView.insertItem(idx, page);
	}

	TPBalloonTip {
		id: exportTypeTip
		imageSource: "export.png"
		message: bShare ? qsTr("What do you want to share?") : qsTr("What to you want to export?")
		button1Text: qsTr("Entire plan")
		button2Text: qsTr("Just this split")
		checkBoxText: qsTr("Human readable?")

		onButton1Clicked: appDB.exportMesoSplit("X", bShare, checkBoxChecked);
		onButton2Clicked: appDB.exportMesoSplit(currentPage.splitModel.splitLetter, bShare, checkBoxChecked);

		property bool bShare: false

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}
} //Page
