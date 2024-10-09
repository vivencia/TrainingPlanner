import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPopup {
	id: dlgExercisesList
	bKeepAbove: true
	width: appSettings.pageWidth
	height: shown ? appSettings.pageHeight * 0.5 : 30
	x: 0
	finalYPos: 0

	property bool shown: false
	property bool bEnableMultipleSelection: false
	property Item currentItemThatRequestedSimpleList: null
	property Item itemThatRequestedSimpleList: null

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
			color: appSettings.paneBackgroundColor
			z: 0
			Layout.fillWidth: true

			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8

			TPMouseArea {
				movableWidget: dlgExercisesList
				movingWidget: recTitleBar
			}

			TPButton {
				id: btnShowHideList
				imageSource: dlgExercisesList.shown ? "fold-up.png" : "fold-down.png"
				hasDropShadow: false
				imageSize: 25
				height: 25
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
				hasDropShadow: false
				imageSize: 20
				z: 2

				anchors {
					right: parent.right
					verticalCenter: parent.verticalCenter
				}

				onClicked: dlgExercisesList.visible = false;
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

			onItemDoubleClicked: dlgExercisesList.visible = false;
		}
	}
}
