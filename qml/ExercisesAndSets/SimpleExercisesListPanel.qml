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

		TPBackRec {
			id: recTitleBar
			height: appSettings.itemDefaultHeight
			Layout.fillWidth: true
			opacity: 0.8

			TPButton {
				id: btnShowHideList
				imageSource: dlgExercisesList.shown ? "fold-up.png" : "fold-down.png"
				hasDropShadow: false
				height: appSettings.itemSmallHeight
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
