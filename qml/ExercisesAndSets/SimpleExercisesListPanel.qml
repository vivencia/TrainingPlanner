import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPopup {
	id: dlgExercisesList
	bKeepAbove: true
	visible: bShowSimpleExercisesList
	width: windowWidth
	height: shown ? windowHeight * 0.5 : 30
	x: 0
	y: 0

	property bool shown: false
	property Item currentItemThatRequestedSimpleList: null

	onVisibleChanged: {
		shown = visible;
		if (shown) {
			navButtons.visible = false;
			focus = true;
			if (currentItemThatRequestedSimpleList !== itemThatRequestedSimpleList) {
				exercisesList.setFilter();
				currentItemThatRequestedSimpleList = itemThatRequestedSimpleList;
			}
			exercisesList.canDoMultipleSelection = bEnableMultipleSelection;
		}
		else
			navButtons.visible = true;
	}

	function hideSimpleExerciseList() {
		bShowSimpleExercisesList = false;
	}

	background: Rectangle {
		id: backRec
		anchors.fill: parent
		color: AppSettings.primaryColor
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

		Rectangle {
			id: recTitleBar
			height: 30
			color: AppSettings.paneBackgroundColor
			z: 0
			Layout.fillWidth: true

			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8

			TPMouseArea {
				movableWidget: dlgExercisesList
				movingWidget: recTitleBar
			}

			TPButton {
				id: btnShowHideList
				imageSource: dlgExercisesList.shown ? "fold-up.png" : "fold-down.png"
				imageSize: 30
				height: 30
				z: 2

				anchors {
					left: parent.left
					verticalCenter: parent.verticalCenter
				}

				onClicked: dlgExercisesList.shown = !dlgExercisesList.shown;
			}

			TPButton {
				id: btnCloseList
				imageSource: "close.png"
				imageSize: 20
				z: 2

				anchors {
					right: parent.right
					verticalCenter: parent.verticalCenter
				}

				onClicked: bShowSimpleExercisesList = false;
			}
		}

		ExercisesListView {
			id: exercisesList
			height: dlgExercisesList.height - 30
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
