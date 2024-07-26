import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Popup {
    id: calendarPopup
	closePolicy: simpleCalendar ? Popup.CloseOnPressOutside : Popup.NoAutoClose
	width: datePickerControl.width
	height: datePickerControl.height
	x: (windowWidth - width) / 3.5 // horizontally centered
	y: (windowHeight - height) / 2 // vertically centered
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	modal: true
	focus: true

	required property date showDate
	required property date initDate
	required property date finalDate

	property bool simpleCalendar: false

	signal dateSelected(date selDate)
	property date selectedDate: datePickerControl.selectedDate

	Rectangle {
		id: backRec
		anchors.fill: parent
		radius: 8
		layer.enabled: true
		visible: false
	}

	background: backRec

	MultiEffect {
		id: backgroundEffect
		visible: true
		source: backRec
		anchors.fill: backRec
		shadowEnabled: true
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1
		opacity: 0.9
	}

	contentItem {
		Keys.onPressed: (event) => {
			if (event.key === mainwindow.backKey) {
				event.accepted = true;
				close();
			}
		}
	}

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
