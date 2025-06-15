import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"

TPPopup {
    id: calendarPopup
	keepAbove: !simpleCalendar
	width: datePickerControl.width
	height: datePickerControl.height + buttonsLayout.childrenRect.height + btnOK.height + 15
	x: (appSettings.pageWidth - width) / 2 // horizontally centered
	finalYPos: (appSettings.pageHeight - height) / 2 // vertically centered

	property date initDate
	property date showDate
	property date finalDate
	property bool simpleCalendar: false
	property bool bInitialized: false

	signal dateSelected(date selDate)

	onKeyboardNumberPressed: (key1, key2) => datePickerControl.setDateByTyping(key1, key2);
	onOpened: datePickerControl.forceActiveFocus();
	onKeyboardEnterPressed: selectDate();

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
		spacing: 5

		TPDatePicker {
			id: datePickerControl
			focus: true
			startDate: initDate
			selectedDate: showDate
			endDate: finalDate
			calendarModel: calModel

			Component.onCompleted: datePickerControl.setDate(showDate);
		}

		Row {
			id: buttonsLayout
			spacing: 2
			Layout.fillWidth: true

			readonly property int buttonWidth: (parent.width-5)/3

			TPButton {
				id: btnYesterday
				text: qsTr("Yesterday")
				flat: false
				width: parent.buttonWidth

				onClicked: datePickerControl.setDate2(appUtils.yesterday());
			}
			TPButton {
				id: btnToday
				text: qsTr("Today")
				flat: false
				width: parent.buttonWidth

				onClicked: datePickerControl.setDate2(appUtils.today());
			}
			TPButton {
				id: btnTomorrow
				text: qsTr("Tomorrow")
				flat: false
				width: parent.buttonWidth

				onClicked: datePickerControl.setDate2(appUtils.tomorrow());
			}
		}

		TPButton {
			id: btnOK
			text: "OK"
			flat: false
			autoSize: true
			Layout.alignment: Qt.AlignCenter

			onClicked: selectDate();
		}
	}

	Component.onCompleted: bInitialized = true;

	function selectDate(): void {
		dateSelected(datePickerControl.selectedDate)
		calendarPopup.closePopup();
	}
}
