import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"

TPPopup {
    id: calendarPopup
	bKeepAbove: !simpleCalendar
	width: datePickerControl.width
	height: datePickerControl.height + 30
	x: (appSettings.pageWidth - width) / 2 // horizontally centered
	finalYPos: (appSettings.pageHeight - height) / 2 // vertically centered

	property date initDate
	property date showDate
	property date finalDate

	property bool simpleCalendar: false

	signal dateSelected(date selDate)

	ColumnLayout {
		anchors.fill: parent
		spacing: 0

		TPDatePicker {
			id: datePickerControl
			focus: true
			startDate: initDate
			displayDate: showDate
			endDate: finalDate
			calendarModel: CalendarModel {
				from: initDate
				to: finalDate
			}

			Component.onCompleted: datePickerControl.setDate(showDate);
		}

		TPButton {
			id: btnOK
			text: "OK"
			flat: false
			Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
			Layout.rightMargin: 5

			onClicked: {
				dateSelected(datePickerControl.selectedDate)
				calendarPopup.close();
			}
		}
	}
}
