import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import com.vivenciasoftware.qmlcomponents

TPPopup {
	id: dialog
	bKeepAbove: true
	width: windowWidth * 0.9
	height: totalHeight + 20

	property string title
	property var importFields: []
	property bool selectedFields: []

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
		source: "import.png"
		width: 50
		height: 50

		anchors {
			top: parent.top
			topMargin: lblTitle.height
			left: parent.left
			leftMargin: 5
		}

		Component.onCompleted: totalHeight += height;
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
			model: importFields.length

			TPCheckBox {
				id: chkImportField
				text: importFields[index]
				checked: true

				onClicked: selectedFields[index] = checked;
			}
		} //Repeater

		Component.onCompleted: totalHeight += repeater.count * 30
	} //ColumnLayout

	RowLayout
	{
		anchors {
			top: fieldsLayout.bottom
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		TPButton {
			id: btnImport
			text: qsTr("Import")
			flat: false
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				importButtonClicked();
				dialog.close();
			}
		}

		TPButton {
			id: btnCancel
			text: qsTr("Cancel")
			flat: false
			Layout.alignment: Qt.AlignCenter

			onClicked: dialog.close();
		}

		Component.onCompleted: totalHeight += 30;
	}
}
