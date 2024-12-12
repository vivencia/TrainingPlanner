import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Frame {
	implicitHeight: label.height + setNotesArea.height + 30
	padding: 0
	spacing: 0

	background: Rectangle {
		color: "transparent"
		border.color: "transparent"
	}

	property string foreColor: "black"
	property alias readOnly: txtNotes.readOnly
	property alias text: txtNotes.text
	property alias info: label.text
	signal editFinished(string new_text)

	ColumnLayout {
		id: notesLayout
		anchors.fill: parent
		spacing: 0

		TPLabel {
			id: label
			text: qsTr("Notes:")
			fontColor: foreColor
			Layout.preferredWidth: _preferredWidth

			TPButton {
				id: button
				imageSource: foreColor + (setNotesArea.visible ? "/fold-up" : "/fold-down")
				hasDropShadow: false
				width: 20
				height: 20

				anchors {
					left: parent.right
					top: parent.top
					leftMargin: 5
				}

				onClicked: setNotesArea.visible = !setNotesArea.visible;
			}
		} //Label

		Flickable {
			id: setNotesArea
			height: visible ? txtNotes.implicitHeight : 30
			width: parent.width - 20
			visible: false
			Layout.fillWidth: true

			TextArea.flickable: TextArea {
				id: txtNotes
				font.pixelSize: appSettings.largeFontSize
				font.bold: true
				padding: 0
				topPadding: 5
				leftPadding: 5
				bottomPadding: 5
				textMargin: 0
				height: 50
				anchors.fill: parent

				onEditingFinished: editFinished(text);
			}

			Component.onCompleted: vBar2.position = 0;
			ScrollBar.vertical: ScrollBar { id: vBar2 }
			ScrollBar.horizontal: ScrollBar {}
		}
	}
}
