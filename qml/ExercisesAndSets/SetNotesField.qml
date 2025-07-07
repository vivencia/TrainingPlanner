import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Column {
	id: control
	padding: 5

	property string info: qsTr("Notes:")
	property string text
	property bool editable

	signal editFinished(string new_text)

	Row {
		Layout.fillWidth: true
		height: appSettings.itemDefaultHeight

		TPLabel {
			id: lblMain
			text: control.info
			width: control.width * 0.9
		}

		TPButton {
			imageSource: setNotesArea.visible ? "fold-up.png" : "fold-down.png"
			hasDropShadow: false
			width: appSettings.itemDefaultHeight*0.8
			height: width

			onClicked: {
				setNotesArea.visible = !setNotesArea.visible;
				if (setNotesArea.visible)
				{
					txtNotes.forceActiveFocus();
					txtNotes.cursorPosition = txtNotes.length;
				}
			}
		}
	}

	ScrollView {
		id: setNotesArea
		contentWidth: availableWidth
		contentHeight: availableHeight
		visible: false
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		width: control.width
		height: appSettings.pageHeight*0.15
		Layout.maximumWidth: width
		Layout.maximumHeight: height

		TextArea {
			id: txtNotes
			text: control.text
			readOnly: !editable
			color: appSettings.fontColor
			font.pixelSize: appSettings.fontSize
			font.bold: true
			topPadding: appSettings.fontSize
			leftPadding: 5
			rightPadding: 5
			bottomPadding: 5
			height: 50

			property bool modified: false

			background: Rectangle {
				color: "white"
				radius: 6
				border.color: appSettings.fontColor
			}

			onTextEdited: modified = true;
			onEditingFinished: {
				if (modified) {
					modified = false;
					editFinished(text);
				}
			}
		}
	}
}
