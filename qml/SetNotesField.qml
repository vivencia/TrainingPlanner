import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
	height: label.height + txtSetNotes.height
	width: parent.width
	Layout.fillWidth: true
	Layout.leftMargin: 5
	Layout.rightMargin: 10
	Layout.bottomMargin: 10

	property alias readOnly: txtNotes.readOnly
	property alias text: txtNotes.text
	property alias color: label.color
	property alias info: label.text
	signal editFinished(string new_text)

	Label {
		id: label
		text: qsTr("Notes:")
		font.bold: true
		anchors {
			left: parent.left
			top: parent.top
		}

		TPRoundButton {
			id: button
			anchors {
				left: parent.right
				verticalCenter: parent.verticalCenter
				rightMargin: 20
			}
			width: 25
			height: 25
			imageName: "fold-down.png"

			onClicked: {
				txtSetNotes.visible = !txtSetNotes.visible;
				imageName = txtSetNotes.visible ? "fold-up.png" : "fold-down.png"
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

		anchors {
			left: parent.left
			top: label.bottom
			right: parent.right
		}

		Component.onCompleted: vBar2.position = 0
		ScrollBar.vertical: ScrollBar { id: vBar2 }
		ScrollBar.horizontal: ScrollBar {}
	}
}
