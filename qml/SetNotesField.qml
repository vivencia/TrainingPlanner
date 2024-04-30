import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
	height: label.height + txtSetNotes.height
	width: parent.width
	Layout.fillWidth: true
	Layout.leftMargin: 5
	Layout.rightMargin: 5
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

		RoundButton {
			id: button
			anchors {
				left: parent.right
				verticalCenter: parent.verticalCenter
				rightMargin: 20
			}
			width: 25
			height: 25

			Image {
				id: img
				source: "qrc:/images/"+darkIconFolder+"fold-down.png"
				width: 20
				height: 20
				anchors {
					verticalCenter: parent.verticalCenter
					horizontalCenter: parent.horizontalCenter
				}
			}

			onClicked: {
				txtSetNotes.visible = !txtSetNotes.visible;
				img.source = txtSetNotes.visible ? "qrc:/images/"+darkIconFolder+"fold-up.png" : "qrc:/images/"+darkIconFolder+"fold-down.png"
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
