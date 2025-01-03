import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../.."
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Column {
	id: mainLayout
	padding: 5
	spacing: 5

	required property TPComplexDialog parentDlg

	Component.onCompleted: parentDlg.dialogOpened.connect(function() {
		mainLayout.height = row.height + checkbox.implicitHeight + 30
	});

	RowLayout {
		id: row
		Layout.leftMargin: 5
		Layout.rightMargin: 5
		Layout.topMargin: 10
		Layout.fillWidth: true
		spacing: 5
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
			heightAvailable: 50
			wrapMode: Text.WordWrap
			horizontalAlignment: Text.AlignJustify
			width: mainLayout.width - imgElement.width - 10
			Layout.preferredWidth: width
		}
	}

	TPCheckBox {
		id: checkbox
		text: parentDlg.customStringProperty2
		checked: false
		width: parent.width
		Layout.leftMargin: 10
		Layout.rightMargin: 5
		Layout.topMargin: 5
		Layout.bottomMargin: 5

		onCheckedChanged: parentDlg.customBoolProperty1 = checked;
	} //TPCheckBox
} // ColumnLayout
