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

				readonly property bool ready: true //the c++ and qml models must have the same API to avoid warnings and errors
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
