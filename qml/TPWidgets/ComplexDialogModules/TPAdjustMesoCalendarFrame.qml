import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

Item {
	id: frmMesoAdjust
	height: chkPreserveOldCalendar.implicitHeight + optPreserveOldCalendar.implicitHeight + optPreserveOldCalendarUntilYesterday.implicitHeight + 20

	required property TPComplexDialog parentDlg

	TPCheckBox {
		id: chkPreserveOldCalendar
		text: qsTr("Preserve previous calendar information?")
		checked: false

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

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

		anchors {
			top: chkPreserveOldCalendar.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 15
			right: parent.right
		}

		onClicked: optPreserveOldCalendarUntilYesterday.checked = false;
		onCheckedChanged: parentDlg.customBoolProperty1 = checked;
	}

	TPRadioButton {
		id: optPreserveOldCalendarUntilYesterday
		text: qsTr("Up until yesterday - ") + appUtils.formatDate(appUtils.getDayBefore(new Date()))
		checked: false
		enabled: chkPreserveOldCalendar.checked

		anchors {
			top: optPreserveOldCalendar.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 15
			right: parent.right
		}

		onClicked: optPreserveOldCalendar.checked = false;
		onCheckedChanged: parentDlg.customBoolProperty2 = checked;
	}
} // ColumnLayout
