import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

TPCheckBox {
	id: chkPreserveOldCalendar
	text: qsTr("Preserve previous calendar information?")
	multiLine: true
	checked: false

	required property TPComplexDialog parentDlg

	onCheckedChanged: parentDlg.customBoolProperty1 = checked;
}
