import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Item {
	id: control
	implicitHeight: lblMain.height + setNotesArea.height + 10

	property string info: qsTr("Notes:")
	property string readOnly
	property string text
	signal editFinished(string new_text)

	TPLabel {
		id: lblMain
		text: control.info
		width: control.width*0.9

		anchors {
			top: control.top
			left: control.left
		}
	}

	TPButton {
		id: button
		imageSource: setNotesArea.visible ? "fold-up.png" : "fold-down.png"
		hasDropShadow: false
		width: 20
		height: 20

		anchors {
			left: lblMain.right
			leftMargin: -(lblMain.width - lblMain._textWidth)
			verticalCenter: lblMain.verticalCenter
		}

		onClicked: {
			setNotesArea.visible = !setNotesArea.visible;
			if (setNotesArea.visible)
			{
				txtNotes.forceActiveFocus();
				txtNotes.cursorPosition = txtNotes.length;
			}
		}
	}

	ScrollView {
		id: setNotesArea
		contentWidth: availableWidth
		visible: false
		height: visible ? appSettings.pageHeight*0.15 : 0
		width: control.width - 20
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded

		TextArea {
			id: txtNotes
			text: control.text
			color: appSettings.fontColor
			font.pixelSize: appSettings.fontSize
			font.bold: true
			topPadding: appSettings.fontSize
			leftPadding: 5
			rightPadding: 5
			bottomPadding: 5
			height: 50

			background: Rectangle {
				color: "white"
				radius: 6
				border.color: appSettings.fontColor
			}

			onEditingFinished: editFinished(text);
		}

		anchors {
			top: lblMain.bottom
			left: control.left
			right: control.right
		}
	}
}
