import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"

TPPopup {
    id: calendarPopup
	bKeepAbove: !simpleCalendar
	width: datePickerControl.width
	height: datePickerControl.height + 30
	x: (windowWidth - width) / 2 // horizontally centered
	finalYPos: (windowHeight - height) / 2 // vertically centered

	required property date showDate
	required property date initDate
	required property date finalDate

	property bool simpleCalendar: false

	signal dateSelected(date selDate)

	ColumnLayout {
		anchors.fill: parent
		spacing: 0

		DatePicker {
			id: datePickerControl
			displayDate: showDate
			startDate: initDate
			endDate: finalDate
			justCalendar: simpleCalendar
			focus: true

			Component.onCompleted: datePickerControl.setDate(showDate);
		}

		Pane {
			height: 25
			Layout.fillWidth: true

			background: Rectangle {
				color: "transparent"
			}

			TPButton {
				id: btnOK
				text: "OK"
				flat: false

				anchors {
					right: parent.right
					rightMargin: 5
					verticalCenter: parent.verticalCenter
				}

				onClicked: {
					dateSelected(datePickerControl.selectedDate)
					calendarPopup.close();
				}
			}

			TPButton {
				text: qsTr("CANCEL")
				flat: false
				visible: !simpleCalendar

				anchors {
					right: btnOK.left
					rightMargin: 30
					verticalCenter: parent.verticalCenter
				}

				onClicked: calendarPopup.close()
			}
		}
	}
}
