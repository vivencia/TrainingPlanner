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
	required property string mesoSplit

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
		anchors.fill: parent
		interactive: !exercisesPane.shown

		onCurrentIndexChanged: currentItem.init();
	} //SwipeView

	PageIndicator {
		count: splitView.count
		currentIndex: splitView.currentIndex
		anchors.bottom: parent.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		visible: !exercisesPane.shown
	}

	footer: ToolBar {
		id: splitToolBar
		width: parent.width
		height: 55
		visible: !bShowSimpleExercisesList

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		ButtonFlat {
			id: btnSave
			text: qsTr("Save")
			imageSource: "qrc:/images/"+lightIconFolder+"save-day.png"
			textUnderIcon: true
			enabled: currentPage ? currentPage.splitModel.modified : false
			fixedSize: true
			width: 80
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

		ButtonFlat {
			id: btnClearPlan
			text: qsTr("Clear")
			imageSource: "qrc:/images/"+lightIconFolder+"clear.png"
			textUnderIcon: true
			enabled: currentPage ? currentPage.splitModel.count > 0 : false
			fixedSize: true
			width: 80
			height: btnAddExercise.height
			anchors.left: btnSave.right
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				currentPage.splitModel.clear();
				currentPage.appendNewExerciseToDivision();
				requestSimpleExercisesList(null, false);
			}
		}

		ButtonFlat {
			id: btnAddExercise
			text: qsTr("Add exercise")
			imageSource: "qrc:/images/"+lightIconFolder+"exercises-add.png"
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
		appDB.getCompleteMesoSplit(mesoSplit);
		if (Qt.platform.os === "android")
			mainwindow.appAboutToBeSuspended.connect(aboutToBeSuspended);
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		itemThatRequestedSimpleList = object;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
	}

	function hideSimpleExerciseList() {
		exercisesPane.shown = false;
	}

	function aboutToBeSuspended() {
		if (currentPage.splitModel.modified)
			btnSave.clicked();
	}
} //Page
