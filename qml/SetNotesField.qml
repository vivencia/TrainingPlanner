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

	property alias readOnly: txtSetNotes.readOnly
	property alias text: txtSetNotes.text
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

	TPTextInput {
		id: txtSetNotes

		anchors {
			left: parent.left
			top: label.bottom
			right: parent.right
		}
		visible: false

		onEditingFinished: editFinished(text);
	}
}
