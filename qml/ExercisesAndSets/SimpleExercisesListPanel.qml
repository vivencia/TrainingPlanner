import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Popup {
	id: bottomPane
	closePolicy: Popup.NoAutoClose
	modal: false
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	visible: bShowSimpleExercisesList
	width: windowWidth
	height: windowHeight * 0.5
	x: 0
	y: shown ? (640 - windowHeight)/2 + height : 640 - 30

	property bool shown: false
	property var currentItemThatRequestedSimpleList: null

	onVisibleChanged: {
		shown = visible;
		if (shown) {
			focus = true;
			if (currentItemThatRequestedSimpleList !== itemThatRequestedSimpleList) {
				exercisesList.setFilter();
				currentItemThatRequestedSimpleList = itemThatRequestedSimpleList;
			}
			exercisesList.canDoMultipleSelection = bEnableMultipleSelection;
		}
	}

	background: Rectangle {
		id: backRec
		anchors.fill: parent
		color: AppSettings.primaryLightColor
		radius: 10
	}

	contentItem {
		Keys.onPressed: (event) => {
			if (event.key === mainwindow.backKey) {
				event.accepted = true;
				close();
			}
		}
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
				imageSize: 30
				height: 30
				onClicked: bottomPane.shown = !bottomPane.shown;
			}
			TPButton {
				id: btnCloseList
				imageSource: "close.png"
				imageSize: 20
				onClicked: bShowSimpleExercisesList = false;
			}
		}

		ExercisesListView {
			id: exercisesList
			height: bottomPane.height - 30
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
