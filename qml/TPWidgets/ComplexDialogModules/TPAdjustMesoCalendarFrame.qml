import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

TPRadioButtonOrCheckBox {
	id: chkPreserveOldCalendar
	text: qsTr("Preserve previous calendar information?")
	radio: false
	multiLine: true
	checked: false

	required property TPComplexDialog parentDlg

	onCheckedChanged: parentDlg.customBoolProperty1 = checked;
}
