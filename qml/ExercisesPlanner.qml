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
	property var pageThatRequestedSimpleList: null

	property alias currentPage: splitView.currentItem

	contentItem {
		Keys.onPressed: (event) => {
			switch (event.key) {
				case Qt.Key_Back:
					if (bottomPane.shown) {
						event.accepted = true;
						bottomPane.shown = false;
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
		interactive: !bottomPane.shown

		onCurrentIndexChanged: currentItem.init();
	} //SwipeView

	PageIndicator {
		count: splitView.count
		currentIndex: splitView.currentIndex
		anchors.bottom: parent.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		visible: !bottomPane.shown
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
				console.log("##################  btnSave.clicked()")
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

	ColumnLayout {
		id: bottomPane
		width: parent.width
		spacing: 0
		visible: bShowSimpleExercisesList
		height: shown ? parent.height * 0.5 : btnShowHideList.height
		property bool shown: true

		onVisibleChanged: shown = visible;
		onShownChanged: {
			if (shown) {
				exercisesList.setFilter();
				exercisesList.canDoMultipleSelection = bEnableMultipleSelection;
			}
		}

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		ButtonFlat {
			id: btnShowHideList
			imageSource: bottomPane.shown ? "qrc:/images/"+darkIconFolder+"fold-down.png" : "qrc:/images/"+darkIconFolder+"fold-up.png"
			imageSize: 60
			onClicked: bottomPane.shown = !bottomPane.shown;
			Layout.fillWidth: true
			Layout.topMargin: 0
			height: 10
			width: bottomPane.width
		}

		ExercisesListView {
			id: exercisesList
			Layout.fillWidth: true
			Layout.topMargin: 5
			Layout.alignment: Qt.AlignTop
			Layout.rightMargin: 5
			Layout.maximumHeight: parent.height - btnShowHideList.height
			Layout.leftMargin: 5
			canDoMultipleSelection: bEnableMultipleSelection

			onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath, multipleSelection_option) => {
				if (pageThatRequestedSimpleList !== null)
					pageThatRequestedSimpleList.changeModel(exerciseName, subName, sets, reps, weight, multipleSelection_option);
			}
		}
	} //ColumnLayout bottomPane

	Component.onCompleted: {
		function insertSplitPage(page, idx) {
			splitView.insertItem(idx, page);
		}

		appDB.getPage.connect(insertSplitPage);
		appDB.getCompleteMesoSplit(mesoSplit);
		if (Qt.platform.os === "android")
			mainwindow.appAboutToBeSuspended.connect(aboutToBeSuspended);
	}

	function requestSimpleExercisesList(object, visible) {
		pageThatRequestedSimpleList = visible ? object : null;
		bShowSimpleExercisesList = visible;
	}

	function hideSimpleExerciseList() {
		bottomPane.shown = false;
	}

	function aboutToBeSuspended() {
		if (currentPage.splitModel.modified)
			btnSave.clicked();
	}
} //Page
