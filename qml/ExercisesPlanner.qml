import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

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
		id: bottomPane
		width: parent.width
		height: shown ? parent.height * 0.5 : btnShowHideList.height
		visible: bShowSimpleExercisesList
		spacing: 0
		padding: 0
		property bool shown: false

		onVisibleChanged: {
			shown = visible;
			if (shown)
				exercisesList.setFilter(pageThatRequestedSimpleList.filterString);
		}

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		background: Rectangle {
			opacity: 0.3
			color: paneBackgroundColor
		}

		ColumnLayout {
			width: parent.width
			height: parent.height
			spacing: 0

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
		}
	} //footer: ToolBar

	Component.onCompleted: {
		appDB.getCompleteMesoSplit(mesoSplit);
	}

	function requestSimpleExerciseList(object, visible) {
		pageThatRequestedSimpleList = visible ? object : null;
		bShowSimpleExercisesList = visible;
	}

	function hideSimpleExerciseList() {
		bottomPane.shown = false;
	}
} //Page
