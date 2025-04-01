import QtQuick
import QtQuick.Controls

import "../"

TextField {
	id: control
	font.pixelSize: appSettings.fontSize
	font.weight: Font.Bold
	color: enabled ? textColor : appSettings.disabledFontColor
	wrapMode: heightAdjustable ? TextInput.WordWrap : TextInput.NoWrap
	verticalAlignment: suggestedHeight === 25 ? Text.AlignVCenter : Text.AlignTop
	validator: RegularExpressionValidator { regularExpression: /^[^#!?&="']*$/ }
	leftInset: 0
	rightInset: 0
	topInset: 0
	bottomInset: 0
	leftPadding: 5
	topPadding: 0
	bottomPadding: 0
	rightPadding: 5
	placeholderTextColor: "gray"
	implicitHeight: suggestedHeight

	property string textColor: appSettings.fontColor
	property string backgroundColor: appSettings.primaryDarkColor
	property bool heightAdjustable: true
	property int suggestedHeight: 25

	readonly property FontMetrics currentFontMetrics: FontMetrics {
		font.family: control.font.family
		font.pixelSize: control.font.pixelSize
		font.weight: control.font.weight
	}

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
		adjustHeight();
		positionCaret();
	}

	onTextEdited: adjustHeight();
	onReadOnlyChanged: positionCaret();
	onWidthChanged: adjustHeight();

	function adjustHeight(): void {
		if (heightAdjustable && width >= 100 && text.length > 20) {
			const textWidth = currentFontMetrics.boundingRect(text).width;
			height = implicitHeight = textWidth > width ? (Math.ceil(textWidth/width) - 1) * suggestedHeight : suggestedHeight;
		}
		else
			height = implicitHeight = suggestedHeight;
	}

	function positionCaret(): void {
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
