import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../.."
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ColumnLayout {
	Layout.fillWidth: true
	required property TPComplexDialog parentDlg

	RowLayout {
		spacing: 5
		Layout.fillWidth: true
		height: Math.max(lblMessage.height, imgElement.height)

		TPImage {
			id: imgElement
			source: parentDlg.customStringProperty3
			visible: parentDlg.customStringProperty3 !== ""
			width: parentDlg.customStringProperty3 !== "" ? 50 : 0
			height: width
			Layout.preferredWidth: width
		}

		TPLabel {
			id: lblMessage
			text: parentDlg.customStringProperty1
			color: textColor
			singleLine: false
			horizontalAlignment: Text.AlignJustify
			width: mainLayout.width - imgElement.width - 10
			Layout.preferredWidth: width
		}
	}

	TPRadioButtonOrCheckBox {
		id: checkbox
		text: parentDlg.customStringProperty2
		radio: false
		checked: false
		Layout.fillWidth: true

		onCheckedChanged: parentDlg.customBoolProperty1 = checked;
	}
} // ColumnLayout
