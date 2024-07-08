import QtQuick
import QtQuick.Layouts

Rectangle {
	id: bottomPane
	width: parent.width
	visible: bShowSimpleExercisesList
	height: shown ? parent.height * 0.5 : btnShowHideList.height
	color: AppSettings.primaryLightColor
	opacity: 0.8
	radius: 10

	property bool shown: true
	property var currentItemThatRequestedSimpleList: null

	onVisibleChanged: {
		shown = visible;
		if (shown) {
			if (currentItemThatRequestedSimpleList !== itemThatRequestedSimpleList) {
				exercisesList.setFilter();
				currentItemThatRequestedSimpleList = itemThatRequestedSimpleList;
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

	ColumnLayout {
		anchors.fill: parent
		spacing: 0

		RowLayout {
			Layout.fillWidth: true
			spacing: 20
			Layout.leftMargin: 10
			Layout.bottomMargin: 2
			Layout.topMargin: 3

			TPButton {
				id: btnShowHideList
				imageSource: bottomPane.shown ? "fold-down.png" : "fold-up.png"
				imageSize: 60
				onClicked: bottomPane.shown = !bottomPane.shown;
			}
			TPButton {
				id: btnCloseList
				imageSource: "close.png"
				imageSize: 60
				onClicked: bShowSimpleExercisesList = false;
			}
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

			onExerciseEntrySelected: {
				if (itemThatRequestedSimpleList)
					itemThatRequestedSimpleList.changeExercise(true);
			}
		}
	}
}
