import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import com.vivenciasoftware.qmlcomponents

Page {
	id: pagePlanner
	objectName: "exercisesPlanner"
	width: windowWidth
	height: windowHeight

	required property int mesoId
	required property int mesoIdx

	property bool bEnableMultipleSelection: false
	property bool bShowSimpleExercisesList: false
	property var itemThatRequestedSimpleList: null

	property alias currentPage: splitView.currentItem

	Keys.onPressed: (event) => {
		if (event.key === Qt.Key_Back) {
			event.accepted = true;
			if (exercisesPane.visible)
				exercisesPane.visible = false;
			else
				pagePlanner.StackView.pop();
		}
	}

	SwipeView {
		id: splitView
		objectName: "splitSwipeView"
		currentIndex: -1
		height: parent.height
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		interactive: !exercisesPane.shown

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
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"save-day.png"
			textUnderIcon: true
			enabled: currentPage ? currentPage.splitModel.modified : false
			fixedSize: true
			width: 55
			height: btnAddExercise.height
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
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"clear.png"
			textUnderIcon: true
			enabled: currentPage ? currentPage.splitModel.count > 1 : false
			fixedSize: true
			width: 55
			height: btnAddExercise.height
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
			text: currentPage ? currentPage.splitModel.splitLetter + " <-> " + currentPage.swappableLetter : "A <-> B"
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"swap.png"
			textUnderIcon: true
			visible: currentPage ? currentPage.bCanSwapPlan : false
			fixedSize: true
			width: 55
			height: btnAddExercise.height
			anchors {
				left: btnClearPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: appDB.swapMesoPlans(currentPage.splitModel.splitLetter, currentPage.swappableLetter);
		}

		TPButton {
			id: btnExportPlan
			text: qsTr("Export")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"export.png"
			textUnderIcon: true
			//visible: currentPage ? currentPage.splitModel.count > 1 : false
			fixedSize: true
			width: 55
			height: btnAddExercise.height
			anchors {
				left: btnSwapPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: exportTypeTip.show(-1);
		}

		TPButton {
			id: btnAddExercise
			text: qsTr("+ Exercise")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"exercises-add.png"
			textUnderIcon: true
			fixedSize: true
			width: 70
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
	}

	Component.onCompleted: {
		function insertSplitPage(page, idx) {
			splitView.insertItem(idx, page);
		}

		appDB.getPage.connect(insertSplitPage);
		appDB.getCompleteMesoSplit();
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		itemThatRequestedSimpleList = object;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
	}

	function hideSimpleExerciseList() {
		exercisesPane.shown = false;
	}

	TPBalloonTip {
		id: exportTypeTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"export.png"
		message: qsTr("What do you want to export?")
		button1Text: qsTr("Entire plan")
		button2Text: qsTr("Just this split")

		onButton1Clicked: exportDialog.init(0);
		onButton2Clicked: exportDialog.init(1);
	}

	TPBalloonTip {
		id: exportTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"export.png"
		button1Text: "OK"

		function init(msg: string) {
			message = msg;
			showTimed(5000, 0);
		}
	}

	FileDialog {
		id: exportDialog
		title: qsTr("Choose the folder and filename to export to")
		currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
		fileMode: FileDialog.SaveFile

		property int _opt

		onAccepted: {
			var result;
			if (_opt === 0) {
				for (var i = 0; i < splitView.count; ++i)
					result = splitView.itemAt(i).splitModel.exportToText(currentFile);
			}
			else
				result = currentPage.splitModel.exportToText(currentFile);
			exportTip.init(result ? qsTr("Meso plan successfully exported") : qsTr("Failed to export meso plan"));
			close();
		}

		function init(opt: int) {
			var suggestedName;
			_opt = opt;
			if (opt === 0)
				suggestedName = qsTr(" - Exercises Plan.tp")
			else
				suggestedName = qsTr(" - Exercises Plan - Split ") + currentPage.splitModel.splitLetter + ".tp";

			currentFile = mesocyclesModel.get(mesoIdx, 1) + suggestedName;
			open();
		}
	}
} //Page
