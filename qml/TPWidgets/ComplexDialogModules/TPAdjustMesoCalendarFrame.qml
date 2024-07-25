import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

Column {
	id: frmMesoAdjust
	padding: 0
	spacing: 0
	height: 120
	Layout.fillWidth: true

	required property var parentDlg

	TPCheckBox {
		id: chkPreserveOldCalendar
		text: qsTr("Preserve previous calendar information?")
		checked: false
		width: frmMesoAdjust.width

		onClicked: {
			if (checked) {
				if (!optPreserveOldCalendar.checked && !optPreserveOldCalendarUntilYesterday.checked)
					optPreserveOldCalendar.checked = true;
			}
			else {
				parentDlg.customBoolProperty1 = false;
				parentDlg.customBoolProperty2 = false;
			}
		}
	} //TPCheckBox

	TPRadioButton {
		id: optPreserveOldCalendar
		text: qsTr("All of the old information")
		enabled: chkPreserveOldCalendar.checked
		checked: false
		Layout.leftMargin: 30

		onCheckedChanged: parentDlg.customBoolProperty1 = checked;
	}

	TPRadioButton {
		id: optPreserveOldCalendarUntilYesterday
		text: qsTr("Up until yesterday - ") + runCmd.formatDate(runCmd.getDayBefore(new Date()))
		checked: false
		enabled: chkPreserveOldCalendar.checked
		Layout.leftMargin: 30

		onCheckedChanged: parentDlg.customBoolProperty2 = checked;
	}
} // ColumnLayout
