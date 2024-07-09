import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls

Popup {
    id: calendarPopup
	closePolicy: simpleCalendar ? Popup.CloseOnPressOutside : Popup.NoAutoClose
	width: datePickerControl.width + 20
	height: datePickerControl.height + 20
	x: (windowWidth - width) / 2 // horizontally centered
	y: (windowHeight - height) / 2 // vertically centered
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	modal: true
	focus: true

	required property date showDate
	required property date initDate
	required property date finalDate

	property bool simpleCalendar: false
	property string windowTitle

	signal dateSelected(date selDate)

	DatePicker {
		id: datePickerControl
		displayDate: showDate
		startDate: initDate
		endDate: finalDate
		justCalendar: simpleCalendar
		calendarWindowTitle: windowTitle

		Component.onCompleted: {
			datePickerControl.setDate(showDate);
		}

		onOkClicked: (selDate) => {
			dateSelected(selDate)
			calendarPopup.close()
		}
		onCancelClicked: {
			calendarPopup.close()
		}
	}
}
