import QtQuick
import QtQuick.Layouts

ColumnLayout {
	id: bottomPane
	width: parent.width
	spacing: 0
	visible: bShowSimpleExercisesList
	height: shown ? parent.height * 0.5 : btnShowHideList.height
	property bool shown: true
	property var currentItemThatRequestedSimpleList: null

	onVisibleChanged: {
		shown = visible;
		if (shown) {
			if (itemThatRequestedSimpleList !== currentItemThatRequestedSimpleList) {
				exercisesList.setFilter();
				itemThatRequestedSimpleList = currentItemThatRequestedSimpleList;
			}
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
		height: windowHeight * 0.8
		Layout.fillWidth: true
		Layout.topMargin: 0
		Layout.alignment: Qt.AlignTop
		Layout.rightMargin: 5
		Layout.maximumHeight: parent.height - btnShowHideList.height
		Layout.leftMargin: 5
		Layout.fillHeight: true
		canDoMultipleSelection: bEnableMultipleSelection

		onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath, multipleSelectionOpt) => {
			if (itemThatRequestedSimpleList)
				itemThatRequestedSimpleList.changeExercise(exerciseName + " - " + subName, sets, reps, weight, multipleSelectionOpt);
		}
	}
}
