import QtQuick
import QtQuick.Controls

import "../"

TextField {
	id: control
	font.pixelSize: appSettings.fontSize
	font.weight: Font.Bold
	color: enabled ? textColor : appSettings.disabledFontColor
	wrapMode: heightAdjustable ? TextInput.WordWrap : TextInput.NoWrap
	validator: enableRegex ? defaultRegEx : null
	leftInset: 0
	rightInset: 0
	topInset: 0
	bottomInset: 0
	leftPadding: 5
	topPadding: 0
	bottomPadding: 0
	rightPadding: defaultPadding
	placeholderTextColor: "gray"
	implicitHeight: suggestedHeight

	property bool showClearTextButton: false
	property bool heightAdjustable: true
	property bool textRemovedKeyPressed: false
	property bool enableRegex: true
	property int suggestedHeight: appSettings.itemDefaultHeight
	property string textColor: appSettings.fontColor
	property string backgroundColor: appSettings.primaryDarkColor
	readonly property int defaultPadding: showClearTextButton ? (text.length > 0 ? btnClearText.width : 0) : 5

	readonly property FontMetrics currentFontMetrics: FontMetrics {
		font.family: control.font.family
		font.pixelSize: control.font.pixelSize
		font.weight: control.font.weight
	}

	signal enterOrReturnKeyPressed()
	signal textCleared()

	onPressAndHold: (event) => {
		event.accepted = true;
		appUtils.copyToClipBoard(selectionStart === selectionEnd ? text : text.substr(selectionStart, selectionEnd));
		mainwindow.showTextCopiedMessage();
	}

	background: Rectangle {
		id: itemBack
		border.color: textColor
		color: control.enabled ? backgroundColor : "transparent"
		radius: 6
		opacity: 0.5
	}

	Keys.onPressed: (event) => {
		textRemovedKeyPressed = false;
		switch (event.key) {
			case Qt.Key_Enter:
			case Qt.Key_Return:
				event.accepted = true;
				enterOrReturnKeyPressed();
			break;
			case Qt.Key_Backspace:
			case Qt.Key_Delete:
				textRemovedKeyPressed = true;
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

	RegularExpressionValidator {
		id: defaultRegEx
		regularExpression: /^[^#!?&="']*$/
	}

	TPButton {
		id: btnClearText
		imageSource: "edit-clear"
		hasDropShadow: false
		visible: showClearTextButton && control.text.length > 0
		width: appSettings.itemDefaultHeight
		height: width

		anchors {
			right: control.right
			rightMargin: 5
			verticalCenter: control.verticalCenter
		}

		onClicked: {
			control.clear();
			control.forceActiveFocus();
			if (readOnly)
				textCleared();
		}
	}

	function adjustHeight(): void {
		if (heightAdjustable && text.length > 20) {
			const textWidth = currentFontMetrics.boundingRect(text).width;
			height = implicitHeight = textWidth > width ? (Math.ceil(textWidth/width) + 1) * suggestedHeight : suggestedHeight;
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
