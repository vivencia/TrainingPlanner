import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

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

	contentItem {
		Keys.onPressed: (event) => {
			switch (event.key) {
				case Qt.Key_Back:
					if (exercisesPane.shown) {
						event.accepted = true;
						exercisesPane.shown = false;
					}
				break;
			}
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
			width: 50
			height: btnAddExercise.height
			anchors.left: parent.left
			anchors.leftMargin: 5
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				appDB.pass_object(currentPage.splitModel);
				appDB.updateMesoSplitComplete(currentPage.splitLetter);
				requestSimpleExercisesList(null, false);
			}
		}

		TPButton {
			id: btnClearPlan
			text: qsTr("Clear")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"clear.png"
			textUnderIcon: true
			enabled: currentPage ? currentPage.splitModel.count > 0 : false
			fixedSize: true
			width: 50
			height: btnAddExercise.height
			anchors.left: btnSave.right
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				currentPage.splitModel.clear();
				currentPage.appendNewExerciseToDivision();
				requestSimpleExercisesList(null, false);
			}
		}

		TPButton {
			id: btnSwapPlan
			text: currentPage ? currentPage.splitLetter + " <-> " + currentPage.swappableLetter : "A <-> B"
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"swap.png"
			textUnderIcon: true
			visible: currentPage ? currentPage.bCanSwapPlan : false
			fixedSize: true
			width: 50
			height: btnAddExercise.height
			anchors.left: btnClearPlan.right
			anchors.verticalCenter: parent.verticalCenter

			onClicked: appDB.swapMesoPlans(currentPage.splitLetter, currentPage.swappableLetter);
		}

		TPButton {
			id: btnAddExercise
			text: qsTr("Add exercise")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"exercises-add.png"
			textUnderIcon: true
			anchors.right: parent.right
			anchors.rightMargin: 5
			anchors.verticalCenter: parent.verticalCenter

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
} //Page
