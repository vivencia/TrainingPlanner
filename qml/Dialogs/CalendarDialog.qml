pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
    id: _control
	keepAbove: !simpleCalendar
	width: datePickerControl.width
	height: mainLayout.childrenRect.height + 15
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
	}

	ColumnLayout {
		id: mainLayout
		spacing: 5

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		TPDatePicker {
			id: datePickerControl
			focus: true
			startDate: _control.initDate
			selectedDate: _control.showDate
			endDate: _control.finalDate
			calendarModel: calModel

			Component.onCompleted: datePickerControl.setDate(_control.showDate);
		}

		TPListView {
			id: calMethodsList
			model: CalendarMethods {
				id: calendarMethods
				date: _control.showDate
			}
			Layout.fillWidth: true
			Layout.preferredHeight: 100

			delegate: ItemDelegate {
				id: delegate
				width: calMethodsList.width
				height: AppSettings.itemLargeHeight

				required property int index
				required property string label

				contentItem: TPLabel {
					text: delegate.label
					elide: Text.ElideMiddle
					wrapMode: Text.NoWrap
					horizontalAlignment: Text.AlignHCenter
				}

				background: Rectangle {
					id:	backgroundColor
					color: AppSettings.primaryLightColor
					radius: 6
					opacity: 1
				}

				onClicked: datePickerControl.setDate(calendarMethods.resultDate(index));
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
