import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPopup {
	id: dlgExercisesList
	keepAbove: true
	width: appSettings.pageWidth
	height: appSettings.pageHeight * 0.5
	x: 0

	onShownChanged: {
		dlgExercisesList.height = shown ? appSettings.pageHeight * 0.5 : recTitleBar.height;
		exercisesList.visible = shown;
	}

	property bool shown: false
	property bool bEnableMultipleSelection: false
	signal exerciseSelected();
	signal listClosed();

	onClosed: listClosed();

	Behavior on height {
		NumberAnimation {
			easing.type: Easing.InOutBack
		}
	}

	ColumnLayout {
		anchors.fill: parent

		Rectangle {
			id: recTitleBar
			height: appSettings.itemDefaultHeight
			color: appSettings.paneBackgroundColor
			Layout.fillWidth: true

			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8

			TPButton {
				id: btnShowHideList
				imageSource: dlgExercisesList.shown ? "fold-up.png" : "fold-down.png"
				hasDropShadow: false
				height: appSettings.itemDefaultHeight*0.9
				width: height

				anchors {
					left: parent.left
					verticalCenter: parent.verticalCenter
				}

				onClicked: dlgExercisesList.shown = !dlgExercisesList.shown;
			}
		}

		ExercisesListView {
			id: exercisesList
			parentPage: dlgExercisesList.parentPage
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.topMargin: 0
			Layout.rightMargin: 5
			Layout.leftMargin: 5
			canDoMultipleSelection: bEnableMultipleSelection

			onExerciseEntrySelected: exerciseSelected();
			onItemDoubleClicked: listClosed();
		}
	}

	function show(ypos: int): void {
		shown = true;
		exercisesList.forceActiveFocus();
		exercisesList.canDoMultipleSelection = bEnableMultipleSelection;
		exercisesModel.clearSelectedEntries();
		show1(ypos);
	}
}
