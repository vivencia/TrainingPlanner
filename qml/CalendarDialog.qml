import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls

Popup {
    id: calendarPopup
	closePolicy: simpleCalendar ? Popup.CloseOnPressOutside : Popup.NoAutoClose

	signal dateSelected(date selDate, int nweek)
	required property date showDate
	required property date initDate
	required property date finalDate
	property bool simpleCalendar

    width: datePickerControl.width + 20
    height: datePickerControl.height + 20
    topMargin: 10
    bottomMargin: 10
    leftMargin: 10
    rightMargin: 10
    // Centered position
	x: parent.x + (parent.width / 2) - (width / 2) // align horizontally centered
    y: parent.y + (parent.height / 2) - (height / 2) // align vertically centered
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates

    modal: true
    focus: true
    clip: true

    DatePicker {
        id: datePickerControl
		displayDate: showDate
		startDate: initDate
		endDate: finalDate
		justCalendar: simpleCalendar

		Component.onCompleted: {
			datePickerControl.setDate (showDate);
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
