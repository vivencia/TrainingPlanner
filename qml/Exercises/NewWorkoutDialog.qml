import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Dialogs
import TpQml.Widgets

TPPopup {
    id: _control
	objectName: "NewWorkoutDlg"
	keepAbove: true
	showTitleBar: true
	normal_size: Qt.size(AppSettings.pageWidth * 0.8, titleBarHeight + datePickerControl.height + 3 * AppSettings.itemLargeHeight)
	open_in_window: true
	savePopupState: true
	resizeable: true

//public:
	required property DBCalendarModel calendarModel
	required property string workoutSplit

	signal dateSelected(selDate: date, calModel: calendarModel)
	signal selectionCalendarChanged(split_workouts: bool)

	onKeyboardNumberPressed: (key1, key2) => datePickerControl.setDateByTyping(key1, key2);
	onOpened: datePickerControl.forceActiveFocus();
	onKeyboardEnterPressed: selectDate();

	onCloseActionExeced: (btn_id) => dateSelected(btn_id === 0 ? datePickerControl.selectedDate : new Date(0,0,0), calendarModel);

	TPLabel {
		id: lblTitle
		text: tr("Choose extra workout date")
		horizontalAlignment: Text.AlignHCenter
		anchors {
			top: _control.contentItem.top
			left: _control.contentItem.left
			right: _control.contentItem.right
			margins: 5
		}
	}

	TPDatePicker {
		id: datePickerControl
		calendarModel: _control.calendarModel
		anchors {
			bottom: lblTitle.bottom
			left: _control.contentItem.left
			right: _control.contentItem.right
			margins: 5
		}
		Component.onCompleted: setDate(_control.showDate);
	}

	TPButtonGroup {
		id: grpDates
	}

	TPRadioButtonOrCheckBox {
		id: optMuscularGroup
		text: tr("Show only ") + _control.workoutSplit + tr("workout days")
		buttonGroup: grpDates
		onClicked: _control.selectionCalendarChanged(true);

		anchors {
			top: datePickerControl.bottom
			left: _control.contentItem.left
			margins: 5
		}
	}
	TPRadioButtonOrCheckBox {
		id: optAnyWorkout
		buttonGroup: grpDates
		text: tr("Any workout day")
		onClicked: _control.selectionCalendarChanged(false);

		anchors {
			top: optMuscularGroup.bottom
			left: _control.contentItem.left
			margins: 5
		}
	}

	TPButton {
		id: btnOK
		text: tr("Schedule workout")
		autoSize: true
		enabled: grpDates.selectedOption >= 0
		anchors {
			top: optAnyWorkout.bottom
			left: _control.contentItem.left
		}

		onClicked: _control.closePopup(0);
	}
	TPButton {
		id: btnCancel
		text: tr("Cancel")
		autoSize: true
		anchors {
			top: optAnyWorkout.bottom
			right: _control.contentItem.right
		}

		onClicked: _control.closePopup(1);
	}
}
