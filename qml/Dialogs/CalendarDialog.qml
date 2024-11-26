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

	property alias initDate: datePickerControl.startDate
	property alias showDate: datePickerControl.displayDate
	property alias finalDate: datePickerControl.endDate
	property alias selectedDate: datePickerControl.selectedDate

	property bool simpleCalendar: false

	signal dateSelected(date selDate)

	ColumnLayout {
		anchors.fill: parent
		spacing: 0

		DatePicker {
			id: datePickerControl
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
