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
		dlgExercisesList.height = shown ? appSettings.pageHeight * 0.5 : titleBar.height;
		exercisesList.visible = shown;
	}

	property bool shown: false
	property bool bEnableMultipleSelection: false
	signal exerciseSelected(Item parentPage);

	Behavior on height {
		NumberAnimation {
			easing.type: Easing.InOutBack
		}
	}

	TPButton {
		imageSource: dlgExercisesList.shown ? "fold-up.png" : "fold-down.png"
		hasDropShadow: false
		width: appSettings.itemDefaultHeight
		height: width
		z: 1

		anchors {
			left: titleBar.left
			leftMargin: 5
			verticalCenter: titleBar.verticalCenter
		}

		onClicked: dlgExercisesList.shown = !dlgExercisesList.shown;
	}

	ExercisesListView {
		id: exercisesList
		parentPage: dlgExercisesList.parentPage
		canDoMultipleSelection: bEnableMultipleSelection

		anchors {
			top: titleBar.bottom
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		onExerciseEntrySelected: exerciseSelected(parentPage);
		onItemDoubleClicked: closePopup();
	}

	function show(ypos: int): void {
		shown = true;
		exercisesList.canDoMultipleSelection = bEnableMultipleSelection;
		exercisesList.setFocusToSearchField();
		exercisesListModel.clearSelectedEntries();
		show1(ypos);
	}
}
