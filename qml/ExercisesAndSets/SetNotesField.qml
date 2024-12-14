import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Item {
	implicitHeight: lblMain.height + setNotesArea.height
	property string foreColor: "black"
	property alias readOnly: txtNotes.readOnly
	property alias text: txtNotes.text
	property alias info: lblMain.text
	signal editFinished(string new_text)

	TPLabel {
		id: lblMain
		text: qsTr("Notes:")
		fontColor: foreColor
		width: parent.width*0.7

		anchors {
			top: parent.top
			left: parent.left
		}
	}

	TPButton {
		id: button
		imageSource: foreColor + (setNotesArea.visible ? "/fold-up" : "/fold-down")
		hasDropShadow: false
		width: 20
		height: 20

		anchors {
			left: lblMain.right
			leftMargin: 5
			verticalCenter: lblMain.verticalCenter
		}

		onClicked: setNotesArea.visible = !setNotesArea.visible;
	}

	ScrollView {
		id: setNotesArea
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		contentWidth: availableWidth
		visible: false
		height: visible ? appSettings.pageHeight*0.15 : 0
		width: parent.width - 20

		TextArea {
			id: txtNotes
			font.pixelSize: appSettings.fontSize
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

		anchors {
			top: lblMain.top
			left: parent.left
			right: parent.right
		}
	}
}
