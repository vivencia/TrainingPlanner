import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../.."
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Item {
	id: mainLayout
	height: row.height + checkbox.implicitHeight + 10

	required property TPComplexDialog parentDlg

	Component.onCompleted: parentDlg.dialogOpened.connect(function() {
		mainLayout.height = row.height + checkbox.implicitHeight + 30
	});

	RowLayout {
		id: row
		spacing: 5
		height: Math.max(lblMessage.height, imgElement.height)

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

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

	TPCheckBox {
		id: checkbox
		text: parentDlg.customStringProperty2
		checked: false

		anchors {
			top: row.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 10
			right: parent.right
		}

		onCheckedChanged: parentDlg.customBoolProperty1 = checked;
	} //TPCheckBox
} // ColumnLayout
