import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../.."
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Column {
	id: mainLayout
	padding: 0
	spacing: 10

	required property var parentDlg

	Component.onCompleted: {
		parentDlg.bAdjustHeightEveryOpen = true;
		parentDlg.dialogOpened.connect(resize);
	}

	RowLayout {
		Layout.leftMargin: 5
		Layout.rightMargin: 5
		Layout.topMargin: 20
		Layout.fillWidth: true
		spacing: 5

		TPImage {
			id: imgElement
			source: parentDlg.customStringProperty3
			visible: parentDlg.customStringProperty3 !== ""
			width: parentDlg.customStringProperty3 !== "" ? 50 : 0
			height: width
		}

		TPLabel {
			id: lblMessage
			text: parentDlg.customStringProperty1
			color: textColor
			wrapMode: Text.WordWrap
			horizontalAlignment: Text.AlignJustify
			width: mainLayout.width - imgElement.width - 10
			Layout.maximumWidth: width
			Layout.minimumWidth: width
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

	function resize() {
		mainLayout.height = Math.max(lblMessage.height, imgElement.height) + checkbox.implicitHeight
	}
} // ColumnLayout
