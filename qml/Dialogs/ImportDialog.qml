import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import com.vivenciasoftware.qmlcomponents

TPPopup {
	id: importDlg
	modal: true
	width: windowWidth * 0.9
	height: totalHeight + 20

	property string title
	property var importOptions: []
	property var selectedFields: []

	property string backColor: AppSettings.primaryColor
	property string textColor: AppSettings.fontColor
	property int totalHeight: 0

	signal importButtonClicked();

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

	Label {
		id: lblTitle
		text: title
		color: textColor
		elide: Text.ElideRight
		horizontalAlignment: Text.AlignHCenter
		font.pointSize: AppSettings.fontSize
		font.weight: Font.Black
		height: 30
		padding: 0

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		Component.onCompleted: totalHeight += height;
	}

	TPImage {
		id: importImg
		source: AppSettings.iconFolder+"import.png"
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

				onClicked: {
					selectedFields[index] = checked;
					if (index === 0) {
						if (importOptions.length > 1) {
							for (var i = 1; i < importOptions.length; i++)
							{
								repeater.itemAt(i).chkImportField.enabled = checked;
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
				importButtonClicked();
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
		importDlg.x = (windowWidth - importDlg.width)/2;

		if (ypos < 0)
			ypos = (windowHeight-importDlg.height)/2;

		finalYPos = ypos;
		if (ypos <= windowHeight/2)
			startYPos = -300;
		else
			startYPos = windowHeight + 300;

		importDlg.open();
	}
}
