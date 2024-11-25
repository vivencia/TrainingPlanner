import QtQuick
import QtQuick.Controls

import "../"

TextField {
	property string textColor: appSettings.fontColor
	property string backgroundColor: appSettings.primaryDarkColor

	id: control
	font.pixelSize: appSettings.fontSize
	font.weight: Font.Bold
	color: enabled ? textColor : appSettings.disabledFontColor
	leftInset: 0
	rightInset: 0
	topInset: 0
	bottomInset: 0
	leftPadding: 5
	topPadding: 0
	bottomPadding: 0
	rightPadding: 5
	placeholderTextColor: "gray"
	implicitWidth: AppGlobals.fontMetricsLarge.boundingRect(text).width + 5
	implicitHeight: 25

	signal enterOrReturnKeyPressed()

	MouseArea {
		pressAndHoldInterval: 800
		enabled: parent.enabled ? true : !parent.readOnly
		anchors.fill: parent

		onPressAndHold: (mouse) => {
			appUtils.copyToClipBoard(control.text);
			mainwindow.showTextCopiedMessage();
		}
		onClicked: (mouse) => {
			mouse.accepted = false;
			control.forceActiveFocus();
		}
		onPressed: (mouse) => {
			mouse.accepted = true;
			control.forceActiveFocus();
		}
	}

	background: Rectangle {
		id: itemBack
		border.color: textColor
		color: backgroundColor
		radius: 6
		opacity: 0.5
	}

	Keys.onPressed: (event) => {
		switch (event.key) {
			case Qt.Key_Enter:
			case Qt.Key_Return:
				event.accepted = true;
				enterOrReturnKeyPressed();
			break;
			default: return;
		}
	}

	onTextChanged: {
		if (readOnly) {
			ensureVisible(0);
			cursorPosition = 0;
		}
	}

	onReadOnlyChanged: {
		if (readOnly) {
			ensureVisible(0);
			cursorPosition = 0;
		}
		else {
			const len = text.length
			ensureVisible(len-1);
			cursorPosition = len;
		}
	}
}
