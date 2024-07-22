import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
	id: frmMesoAdjust
	Layout.fillWidth: true
	Layout.rightMargin: 20
	Layout.leftMargin: 5
	visible: mesoCalendarModel.count === 0 ? false : bStartDateChanged || bEndDateChanged || bMesoSplitOK
	padding: 0
	spacing: 0
	height: 300

	background: Rectangle {
		border.color: AppSettings.fontColor
		color: "transparent"
		radius: 6
	}

	ColumnLayout {
		id: layoutSplit
		anchors.fill: parent
		spacing: 0

		TPCheckBox {
			id: chkPreserveOldCalendar
			text: qsTr("Preserve previous calendar information?")
			checked: false
			Layout.fillWidth: true
			Layout.leftMargin: 5

			onClicked: {
				if (checked) {
					if (!optPreserveOldCalendar.checked && !optPreserveOldCalendarUntilYesterday.checked)
						optPreserveOldCalendar.checked = true;
				}
			}
		} //TPCheckBox

		TPRadioButton {
			id: optPreserveOldCalendar
			text: qsTr("All of the old information")
			enabled: chkPreserveOldCalendar.checked
			checked: false
		}

		TPRadioButton {
			id: optPreserveOldCalendarUntilYesterday
			text: qsTr("Up until yesterday - ") + runCmd.formatDate(runCmd.getDayBefore(new Date()));
			checked: false
			enabled: chkPreserveOldCalendar.checked //&& isDateWithinMeso(today)
		}
	} // ColumnLayout
} //Frame
