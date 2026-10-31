pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Dialogs
import TpQml.Widgets

TPPopup {
    id: _control
	keepAbove: !simpleCalendar
	minimum_size: Qt.size(datePickerControl.minimumWidth, datePickerControl.minimumHeight + 100 + AppSettings.itemDefaultHeight)
	width: minimum_size.width
	height: minimum_size.height
	open_in_window: true
	show_position: Qt.AlignCenter
	resizeable: true
	mouseItem: bottomBar

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
			datePickerControl.qtCalendarModel = 0;
			calModel.from = initDate;
			datePickerControl.qtCalendarModel = calModel;
		}
	}
	onFinalDateChanged: {
		if (bInitialized) {
			datePickerControl.qtCalendarModel = 0;
			calModel.to = finalDate;
			datePickerControl.qtCalendarModel = calModel;
		}
	}

	CalendarModel {
		id: calModel
		from: _control.initDate
		to: _control.finalDate
	}

	TPDatePicker {
		id: datePickerControl
		focus: true
		startDate: _control.initDate
		selectedDate: _control.showDate
		endDate: _control.finalDate
		qtCalendarModel: calModel
		height: (_control.height - bottomBar.height) * 0.65

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		Component.onCompleted: datePickerControl.setDate(_control.showDate);
	}

	ListView {
		id: calMethodsList
		delegateModelAccess: DelegateModel.ReadOnly
		spacing: 2
		height: (_control.height - bottomBar.height) * 0.3

		model: CalendarMethods {
			id: calendarMethods
			date: _control.showDate
		}

		anchors {
			top: datePickerControl.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}

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
				color: delegate.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2
				radius: 8
			}

			onClicked: datePickerControl.setDate(calendarMethods.resultDate(index));
		}
	}

	Rectangle {
		id: bottomBar
		color: AppSettings.paneBackgroundColor
		radius: 8
		height: AppSettings.itemDefaultHeight
		width: parent.width
		anchors.bottom: parent.bottom

		TPButton {
			id: btnOK
			text: "OK"
			flat: true

			anchors {
				verticalCenter: parent.verticalCenter
				horizontalCenter: parent.horizontalCenter
			}
			onClicked: _control.selectDate();
		}
	}

	Component.onCompleted: bInitialized = true;

	function selectDate(): void {
		dateSelected(datePickerControl.selectedDate)
		_control.closePopup(0);
	}
}
