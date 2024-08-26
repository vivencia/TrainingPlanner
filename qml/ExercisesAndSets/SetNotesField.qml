import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

ColumnLayout {
	height: label.height + txtSetNotes.visible ? txtSetNotes.height : 0
	implicitHeight: height
	width: parent.width
	Layout.fillWidth: true
	Layout.leftMargin: 5
	spacing: 0

	property alias readOnly: txtNotes.readOnly
	property alias text: txtNotes.text
	property alias color: label.color
	property alias info: label.text
	signal editFinished(string new_text)

	Label {
		id: label
		text: qsTr("Notes:")
		font.bold: true
		height: 20
		Layout.alignment: Qt.AlignLeft
		Layout.fillWidth: false
		padding: 0

		TPButton {
			id: button
			imageSource: "fold-down.png"

			anchors {
				left: parent.right
				top: parent.top
				topMargin: -5
				rightMargin: 20
			}

			onClicked: {
				txtSetNotes.visible = !txtSetNotes.visible;
				imageSource = txtSetNotes.visible ? "fold-up.png" : "fold-down.png"
			}
		}
	} //Label

	Flickable {
		id: txtSetNotes
		height: Math.min(contentHeight, 40)
		width: parent.width - 20
		contentHeight: txtNotes.implicitHeight
		topMargin: 0
		leftMargin: 0
		rightMargin: 0
		bottomMargin: 0
		visible: false
		Layout.fillWidth: true

		TextArea.flickable: TextArea {
			id: txtNotes
			font.pointSize: AppSettings.fontSizeText
			font.bold: true
			padding: 0
			topPadding: 5
			leftPadding: 5
			bottomPadding: 5
			textMargin: 0
			onEditingFinished: editFinished(text);
		}

		Component.onCompleted: vBar2.position = 0
		ScrollBar.vertical: ScrollBar { id: vBar2 }
		ScrollBar.horizontal: ScrollBar {}
	}
}
