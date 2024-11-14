import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Frame {
	height: label.height + txtSetNotes.height
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
		anchors.fill: parent
		spacing: 0

		TPLabel {
			id: label
			text: qsTr("Notes:")
			fontColor: foreColor
			height: 20
			Layout.alignment: Qt.AlignLeft
			Layout.fillWidth: false

			TPButton {
				id: button
				imageSource: foreColor + (txtSetNotes.visible ? "/fold-up" : "/fold-down")
				hasDropShadow: false
				width: 20
				height: 20

				anchors {
					left: parent.right
					top: parent.top
					rightMargin: 20
				}

				onClicked: txtSetNotes.visible = !txtSetNotes.visible;
			}
		} //Label

		Flickable {
			id: txtSetNotes
			height: visible ? txtNotes.implicitHeight : 0
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
