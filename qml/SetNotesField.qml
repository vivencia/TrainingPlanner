import QtQuick
import QtQuick.Controls

Item {
	height: label.height + txtSetNotes.height

	Label {
		id: label
		text: qsTr("Notes:")
		font.bold: true
		padding: 2
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

	TextField {
		id: txtSetNotes
		anchors {
			left: parent.left
			top: label.bottom
			right: parent.right
			rightMargin: 5
			leftMargin: 5
			bottomMargin: 10
		}
		visible: false

		onTextEdited: tDayModel.setSetNotes(text, exerciseIdx);
		Component.onCompleted: text = tDayModel.setNotes(setNumber, exerciseIdx);
	}
}
