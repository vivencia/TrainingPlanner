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
	height: totalHeight + 20

	property var importOptions: []
	property var selectedFields: []

	property string backColor: appSettings.primaryColor
	property string textColor: appSettings.fontColor
	property int totalHeight: 0

	TPButton {
		imageSource: "close.png"
		hasDropShadow: false
		height: 30
		width: 30

		z:2

		anchors {
			top: parent.top
			right: parent.right
		}

		onClicked: close();
	}

	TPLabel {
		id: lblTitle
		text: qsTr("Try to import?")
		color: textColor
		elide: Text.ElideRight
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		Component.onCompleted: totalHeight += height;
	}

	TPImage {
		id: importImg
		source: appSettings.iconFolder+"import.png"
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
		}

		Repeater {
			id: repeater
			model: importOptions.length

			TPCheckBox {
				id: chkImportField
				text: importOptions[index]
				checked: true
				width: parent.width

				onClicked: {
					selectedFields[index] = checked;
					if (index === 0) {
						if (importOptions.length > 1) {
							for (let i = 1; i < importOptions.length; i++) {
								repeater.itemAt(i).children[0].enabled = checked;
								selectedFields[i] = checked;
							}
						}
					}
				}
			}
		} //Repeater

		Component.onCompleted: totalHeight += repeater.count * 30
	} //ColumnLayout

	RowLayout
	{
		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			bottomMargin: 10
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

		Component.onCompleted: totalHeight += 30;
	}

	function show(ypos) {
		importDlg.x = (appSettings.pageWidth - importDlg.width)/2;

		if (ypos < 0)
			ypos = (appSettings.pageHeight-importDlg.height)/2;

		finalYPos = ypos;
		if (ypos <= appSettings.pageHeight/2)
			startYPos = -300;
		else
			startYPos = appSettings.pageHeight + 300;

		importDlg.open();
	}
}
