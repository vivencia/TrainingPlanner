import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: importDlg
	modal: true
	width: appSettings.pageWidth * 0.9

	property list<string> importOptions
	property list<bool> selectedFields

	TPLabel {
		id: lblTitle
		text: qsTr("Try to import?")
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}
	}

	TPImage {
		id: importImg
		source: "import.png"
		width: 50
		height: 50

		anchors {
			top: parent.top
			topMargin: lblTitle.height
			left: parent.left
			leftMargin: 5
		}
	}

	ColumnLayout
	{
		id: fieldsLayout
		spacing: 0

		anchors {
			top: lblTitle.bottom
			left: importImg.right
			right: parent.right
			rightMargin: 5
		}

		Repeater {
			id: repeater
			model: importOptions.length

			property int itemsHeight: 0

			TPCheckBox {
				id: chkImportField
				text: importOptions[index]
				checked: selectedFields[index]
				width: parent.width

				required property int index
				onClicked: selectedFields[index] = checked;

				Component.onCompleted: {
					if (index === 0)
						repeater.itemsHeight = 0;
					repeater.itemsHeight += height;
				}
			}
		} //Repeater
	} //ColumnLayout

	RowLayout
	{
		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			bottomMargin: 5
		}

		TPButton {
			id: btnImport
			text: qsTr("Import")
			flat: false
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				itemManager.tryToImport(selectedFields);
				importDlg.close();
			}
		}

		TPButton {
			id: btnCancel
			text: qsTr("Cancel")
			flat: false
			Layout.alignment: Qt.AlignCenter

			onClicked: importDlg.close();
		}
	}

	function show(ypos): void {
		importDlg.height = 0;
		importDlg.height = Math.max(height + importImg.height) + repeater.itemsHeight + btnImport.height + 30;
		show1(ypos);
	}
}
