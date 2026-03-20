import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
    id: _control
	keepAbove: !simpleCalendar
	width: datePickerControl.width
	height: datePickerControl.height + buttonsLayout.childrenRect.height + btnOK.height + 15
	x: (AppSettings.pageWidth - width) / 2 // horizontally centered
	finalYPos: (AppSettings.pageHeight - height) / 2 // vertically centered

//public:
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
		from: _control.initDate
		to: _control.finalDate

		readonly property bool ready: true //the c++ and qml models must have the same API to avoid warnings and errors
	}

	ColumnLayout {
		anchors.fill: parent
		spacing: 5

		TPDatePicker {
			id: datePickerControl
			focus: true
			startDate: _control.initDate
			selectedDate: _control.showDate
			endDate: _control.finalDate
			calendarModel: calModel

			Component.onCompleted: datePickerControl.setDate(_control.showDate);
		}

		Row {
			id: buttonsLayout
			spacing: 2
			Layout.fillWidth: true

			readonly property int buttonWidth: (parent.width - 5) / 3

			TPButton {
				id: btnYesterday
				text: qsTr("Yesterday")
				width: parent.buttonWidth

				onClicked: datePickerControl.setDate2(AppUtils.yesterday());
			}
			TPButton {
				id: btnToday
				text: qsTr("Today")
				width: parent.buttonWidth

				onClicked: datePickerControl.setDate2(AppUtils.today());
			}
			TPButton {
				id: btnTomorrow
				text: qsTr("Tomorrow")
				width: parent.buttonWidth

				onClicked: datePickerControl.setDate2(AppUtils.tomorrow());
			}
		}

		TPButton {
			id: btnOK
			text: "OK"
			autoSize: true
			Layout.alignment: Qt.AlignCenter

			onClicked: _control.selectDate();
		}
	}

	Component.onCompleted: bInitialized = true;

	function selectDate(): void {
		dateSelected(datePickerControl.selectedDate)
		_control.closePopup();
	}
}
