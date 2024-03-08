import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls

Popup {
    id: calendarPopup
	closePolicy: simpleCalendar ? Popup.CloseOnPressOutside : Popup.NoAutoClose
	width: datePickerControl.width + 20
	height: datePickerControl.height + 20
	topMargin: 10
	bottomMargin: 10
	leftMargin: 10
	rightMargin: 10
	// Centered position
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

	signal dateSelected(date selDate, int nweek)

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

		onOkClicked: (selDate, nWeek) => {
			dateSelected(selDate, nWeek)
			calendarPopup.close()
		}
		onCancelClicked: {
			calendarPopup.close()
		}
	}
}
