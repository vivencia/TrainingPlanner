import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Item {
	id: control
	implicitHeight: lblMain.height + setNotesArea.height + 10

	property string foreColor: "black"
	property string info: qsTr("Notes:")
	property string readOnly
	property string text
	signal editFinished(string new_text)

	TPLabel {
		id: lblMain
		text: control.info
		fontColor: control.foreColor
		width: control.width*0.9

		anchors {
			top: control.top
			left: control.left
		}
	}

	TPButton {
		id: button
		imageSource: {
			let strColor = control.foreColor
			if (strColor === appSettings.fontColor)
				strColor = appSettings.iconFolder;
			else
				strColor = strColor + '/';
			return strColor + (setNotesArea.visible ? "fold-up" : "fold-down")
		}
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
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		contentWidth: availableWidth
		visible: false
		height: visible ? appSettings.pageHeight*0.15 : 0
		width: control.width - 20

		TextArea {
			id: txtNotes
			text: control.text
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
				border.color: control.foreColor
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
