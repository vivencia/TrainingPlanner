import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import "../TPWidgets"

TPPopup {
    id: calendarPopup
	bKeepAbove: !simpleCalendar
	width: datePickerControl.width
	height: datePickerControl.height
	x: (windowWidth - width) / 2 // horizontally centered
	y: (windowHeight - height) / 2 // vertically centered

	required property date showDate
	required property date initDate
	required property date finalDate

	property bool simpleCalendar: false

	signal dateSelected(date selDate)
	property date selectedDate: datePickerControl.selectedDate

	DatePicker {
		id: datePickerControl
		displayDate: showDate
		startDate: initDate
		endDate: finalDate
		justCalendar: simpleCalendar

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
