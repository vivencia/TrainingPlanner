import QtQuick

import TpQml
import TpQml.Widgets

TPPopup {
	id: _control
	keepAbove: true
	width: AppSettings.pageWidth
	height: AppSettings.pageHeight * 0.5
	x: 0

	onShownChanged: {
		_control.height = shown ? AppSettings.pageHeight * 0.5 : titleBar.height;
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
		imageSource: _control.shown ? "fold-up.png" : "fold-down.png"
		hasDropShadow: false
		width: AppSettings.itemDefaultHeight
		height: width
		z: 1

		anchors {
			left: _control.titleBar.left
			leftMargin: 5
			verticalCenter: _control.titleBar.verticalCenter
		}

		onClicked: _control.shown = !_control.shown;
	}

	ExercisesListView {
		id: exercisesList
		parentPage: _control.parentPage
		enableMultipleSelection: _control.bEnableMultipleSelection

		anchors {
			top: _control.titleBar.bottom
			left: _control.contentItem.left
			right: _control.contentItem.right
			bottom: _control.contentItem.bottom
		}

		onExerciseEntrySelected: _control.exerciseSelected(_control.parentPage);
		onItemDoubleClicked: _control.closePopup();
	}

	function show(ypos: int): void {
		shown = true;
		exercisesList.enableMultipleSelection = bEnableMultipleSelection;
		exercisesList.setFocusToSearchField();
		AppExercisesList.clearSelectedEntries();
		show1(ypos);
	}
}
