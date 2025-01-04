import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"

TPPopup {
    id: calendarPopup
	keepAbove: !simpleCalendar
	width: datePickerControl.width
	height: datePickerControl.height + 30
	x: (appSettings.pageWidth - width) / 2 // horizontally centered
	finalYPos: (appSettings.pageHeight - height) / 2 // vertically centered

	property date initDate
	property date showDate
	property date finalDate
	property bool simpleCalendar: false
	property bool bInitialized: false

	signal dateSelected(date selDate)

	onInitDateChanged: {
		if (bInitialized) {
			datePickerControl.calendarModel = 0;
			calModel.from = initDate;
			datePickerControl.calendarModel = calModel;
		}
	}
	onFinalDateChanged: {
		if (bInitialized) {
			datePickerControl.calendarModel = 0;
			calModel.to = finalDate;
			datePickerControl.calendarModel = calModel;
		}
	}

	CalendarModel {
		id: calModel
		from: initDate
		to: finalDate

		readonly property bool ready: true //the c++ and qml models must have the same API to avoid warnings and errors
	}

	ColumnLayout {
		anchors.fill: parent
		spacing: 0

		TPDatePicker {
			id: datePickerControl
			focus: true
			startDate: initDate
			displayDate: showDate
			endDate: finalDate
			calendarModel: calModel

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
				calendarPopup.closePopup();
			}
		}
	}

	Component.onCompleted: bInitialized = true;
}
