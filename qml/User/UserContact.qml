import QtQuick
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"

Frame {
	property bool bReady: readyBlocks[0] & readyBlocks[1] & readyBlocks[2]
	property var readyBlocks: [false,false,false]
	readonly property int nControls: 5
	readonly property int controlsHeight: 25
	readonly property int allControlsHeight: nControls*controlsHeight
	readonly property int controlsSpacing: 10

	Label {
		id: lblPhone
		text: userModel.columnLabel(4)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		bottomPadding: 0

		anchors {
			top: parent.top
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtPhone
		height: controlsHeight
		inputMethodHints: Qt.ImhDigitsOnly
		inputMask: "+55 \\ (99\\) 99999\\-9999;_"
		ToolTip.text: qsTr("Invalid phone number")

		Component.onCompleted: {
			text = userModel.phone;
			readyBlocks[0] = !userModel.isEmpty();
		}

		onActiveFocusChanged: {
			if (activeFocus) {
				if (text.length < 20)
					cursorPosition = 5;
				else
					cursorPosition = 20;
			}
		}

		onTextChanged: userModel.phone = text;

		onEnterOrReturnKeyPressed: {
			if (txtEmail.enabled)
				txtEmail.forceActiveFocus();
		}

		onTextEdited: {
			if (text.length === 20) {
				txtEmail.enabled = true;
				ToolTip.visible = false;
				readyBlocks[0] = true;
			}
			else {
				txtEmail.enabled = false;
				ToolTip.visible = true;
				readyBlocks[0] = false;
			}
			bReady = readyBlocks[0] & readyBlocks[1] & readyBlocks[2];
		}

		anchors {
			top: lblPhone.bottom
			topMargin: -10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	Label {
		id: lblEmail
		text: userModel.columnLabel(5)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0

		anchors {
			top: txtPhone.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtEmail
		height: controlsHeight
		enabled: !userModel.isEmpty()
		ToolTip.text: qsTr("Invalid email address")

		Component.onCompleted: {
			text = userModel.email;
			readyBlocks[1] = !userModel.isEmpty();
		}

		onTextChanged: userModel.email = text;

		onTextEdited: {
			if (text.length > 0) {
				if (text.indexOf("@") !== -1) {
					if (text.indexOf(".") !== -1) {
						txtSocial.enabled = true;
						ToolTip.visible = false;
						readyBlocks[1] = true;
						return;
					}
				}
			}
			txtSocial.enabled = false;
			ToolTip.visible = true;
			readyBlocks[1] = false;
			bReady = readyBlocks[0] & readyBlocks[1] & readyBlocks[2];
		}

		anchors {
			top: lblEmail.bottom
			topMargin: -10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	Label {
		id: lblSocial
		text: userModel.columnLabel(6)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0

		anchors {
			top: txtEmail.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtSocial
		height: controlsHeight
		enabled: !userModel.isEmpty()
		ToolTip.text: qsTr("Social media address is too short")

		Component.onCompleted: {
			text = userModel.socialMedia;
			readyBlocks[2] = !userModel.isEmpty();
		}

		onTextChanged: userModel.socialMedia = text;

		onTextEdited: {
			readyBlocks[2] = text.length > 5 || text.length === 0
			ToolTip.visible = readyBlocks[2];
			bReady = readyBlocks[0] & readyBlocks[1] & readyBlocks[2];
		}

		anchors {
			top: lblSocial.bottom
			topMargin: -10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}
}
