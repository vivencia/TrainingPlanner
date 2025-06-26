import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets/"

FocusScope {
	id: control
	implicitWidth: availableWidth
	width: availableWidth
	implicitHeight: 30
	height: visible ? implicitHeight : 0

	required property int type
	required property int availableWidth

	property alias text: txtMain.text
	property bool showLabel: true
	property bool showButtons: true
	property bool clearInput: true
	property bool editable: true
	property list<string> labelText: [ qsTr("Weight") + appSettings.weightUnit + ':', qsTr("Reps:"), qsTr("Rest time:"), qsTr("SubSets:") ]
	property color borderColor: appSettings.fontColor
	property color labelColor: appSettings.fontColor
	property color inputColor: appSettings.fontColor
	property color backColor: appSettings.paneBackgroundColor

	readonly property list<QtObject> validatorType: [val_weigth, val_rep, val_time, val_set]
	readonly property list<int> maxLen: [5,4,5,1]
	property string origText

	signal valueChanged(string str)
	signal enterOrReturnKeyPressed()

	enum Type {
		WeightType,
		RepType,
		TimeType,
		SetType
	}

	DoubleValidator {
		id: val_weigth
		bottom: 0.00
		top: 999.99;
		decimals: 2
		notation: DoubleValidator.StandardNotation
		locale: appSettings.appLocale
	}

	DoubleValidator {
		id: val_rep
		bottom: 0.00
		top: 99.99;
		decimals: 2
		notation: DoubleValidator.StandardNotation
		locale: appSettings.appLocale
	}

	RegularExpressionValidator {
		id: val_time
		regularExpression: /\^[0-5][0-9][:][0-5][0-9]$/
	}

	IntValidator {
		id: val_set
		bottom: 0
		top: 9
	}

	Rectangle {
		anchors.fill: parent
		border.color: borderColor
		radius: 6
		color: control.enabled ? backColor : "transparent"

		TPLabel {
			id: lblMain
			text: labelText[type]
			fontColor: labelColor
			visible: showLabel

			anchors {
				left: parent.left
				leftMargin: 2
				verticalCenter: parent.verticalCenter
			}
		}

		TPButton {
			id: btnIncreaseMinutes
			imageSource: "plus"
			hasDropShadow: false
			width: appSettings.itemDefaultHeight*0.9
			height: width
			visible: showButtons && type === SetInputField.Type.TimeType
			enabled: editable

			anchors {
				left: showLabel ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				txtMain.text = appUtils.addTimeToStrTime(txtMain.text, 1, 0)
				valueChanged(txtMain.text);
			}
		}

		TPButton {
			id: btnDecrease
			imageSource: "minus"
			hasDropShadow: false
			width: appSettings.itemDefaultHeight*0.9
			height: width
			visible: showButtons
			enabled: editable

			anchors {
				left: btnIncreaseMinutes.visible ? btnIncreaseMinutes.right : showLabel ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				clearInput = false;
				txtMain.text = appUtils.setTypeOperation(type, false, txtMain.text !== "" ? txtMain.text : origText)
				valueChanged(txtMain.text);
			}
		}

		TPTextInput {
			id: txtMain
			heightAdjustable: false
			validator: validatorType[type]
			inputMethodHints: type <= SetInputField.Type.RepType ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly
			maximumLength: maxLen[type]
			readOnly: !editable
			padding: 0
			focus: true

			width: {
				switch (type) {
					case SetInputField.WeightType:
						return Math.min(availableWidth*0.35, 2*height);
					case SetInputField.RepType:
						return Math.min(availableWidth*0.25, 1.5*height);
					case SetInputField.Type.TimeType:
						return Math.min(availableWidth*0.4, 3*height);
					case SetInputField.SetType:
						return Math.min(availableWidth*0.2, height);
				}
			}

			anchors {
				left: showButtons ? btnDecrease.right : showLabel ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onEnterOrReturnKeyPressed: {
				clearInput = true;
				control.enterOrReturnKeyPressed();
			}

			onActiveFocusChanged: {
				if (activeFocus) {
					if (clearInput) {
						origText = text;
						txtMain.clear();
						clearInput = false; //In case the window loose focus, when returning do not erase what was being written before the loosing of focus
					}
				}
				else {
					if (text === "")
						text = origText;
				}
			}

			onTextEdited: {
				if (type === SetInputField.Type.TimeType) {
					let oldText = text;
					let oldCursor = cursorPosition;
					let digits = oldText.replace(/\D/g, '');
					text = formatTime(digits);
					cursorPosition = adjustCursorPosition(oldText, text, oldCursor);
				}
			}
			onTextChanged: if (!activeFocus) clearInput = true;
			onEditingFinished: valueChanged(text = sanitizeText(text));
		} //TextInput

		TPButton {
			id: btnIncrease
			imageSource: "plus"
			hasDropShadow: false
			width: appSettings.itemDefaultHeight*0.9
			height: width
			visible: showButtons
			enabled: editable

			anchors {
				left: txtMain.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				clearInput = false;
				txtMain.text = appUtils.setTypeOperation(type, true, txtMain.text !== "" ? txtMain.text : origText)
				valueChanged(txtMain.text);
			}
		}

		TPButton {
			id: btnDecreaseSeconds
			imageSource: "minus"
			hasDropShadow: false
			width: appSettings.itemDefaultHeight*0.9
			height: width
			visible: showButtons && type === SetInputField.Type.TimeType
			enabled: editable

			anchors {
				left: btnIncrease.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				const secs = parseInt(txtMain.text.substring(3, 5));
				const nbr = secs > 5 ? -5 : -1;
				txtMain.text = appUtils.addTimeToStrTime(txtMain.text, 0, nbr);
				valueChanged(txtMain.text);
			}
		}
	} //Rectangle

	function sanitizeText(text) : void {
		text = text.replace('.', ',');
		text = text.replace('-', '');
		text = text.replace('E', '');
		return text.trim();
	}

	function formatTime(digits: string) : string {
		// Remove all non-digits
		digits = digits.replace(/\D/g, '');

		// Format based on length
		let formatted = "";
		if (digits.length === 0)
			return ""; // Empty input shows nothing
		else if (digits.length <= 2)
			formatted = digits + ":";
		else
			formatted = digits.substring(0, 2) + ":" + digits.substring(2)
		return formatted;
	}

	// Function to calculate cursor position after formatting
	function adjustCursorPosition(oldText: string, newText: string, oldCursor: int) : void {
		// Count non-digit characters (formatting chars) up to old cursor position
		let nonDigitsBeforeCursor = oldText.substring(0, oldCursor).replace(/[0-9]/g, '').length;
		// Count digits up to old cursor position
		let digitsBeforeCursor = oldText.substring(0, oldCursor).replace(/\D/g, '').length;
		// Calculate new cursor position in formatted text
		let newFormatted = formatTime(newText.replace(/\D/g, ''));
		let digitCount = 0;
		let nonDigitCount = 0;
		for (let i = 0; i < newFormatted.length; i++) {
			if (/\d/.test(newFormatted[i])) {
				digitCount++;
				if (digitCount === digitsBeforeCursor) {
					return i + 1; // Place cursor after the current digit
				}
			} else {
				nonDigitCount++;
			}
		}
		return newFormatted.length; // Fallback to end of text
	}
} //FocusScope
