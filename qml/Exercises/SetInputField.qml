import QtQuick

import TpQml
import TpQml.Widgets

FocusScope {
	id: _control
	implicitWidth: availableWidth
	width: availableWidth
	implicitHeight: AppSettings.itemDefaultHeight
	height: visible ? implicitHeight : 0

	required property int type
	required property int availableWidth

	property alias text: txtMain.text
	property bool showLabel: true
	property bool showButtons: true
	property bool clearInput: true
	property bool editable: true
	property string borderColor: AppSettings.fontColor
	property string labelColor: AppSettings.fontColor
	property string inputColor: AppSettings.fontColor
	property string backColor: AppSettings.paneBackgroundColor

	readonly property list<string> labelText: [ qsTr("Weight") + AppSettings.weightUnit + ':', qsTr("Reps:"), qsTr("Rest time:"), qsTr("SubSets:") ]
	readonly property list<QtObject> validatorType: [val_weigth, val_rep, val_time, val_set]
	readonly property list<int> maxLen: [5,4,5,1]

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
		locale: AppSettings.userLocale
	}

	DoubleValidator {
		id: val_rep
		bottom: 0.00
		top: 99.99;
		decimals: 2
		notation: DoubleValidator.StandardNotation
		locale: AppSettings.userLocale
	}

	RegularExpressionValidator {
		id: val_time
		regularExpression: /^[0-5][0-9]?:[0-5][0-9]$/
	}

	IntValidator {
		id: val_set
		bottom: 0
		top: 9
	}

	Rectangle {
		anchors.fill: parent
		border.color: _control.borderColor
		radius: 6
		color: _control.enabled ? _control.backColor : "transparent"

		TPLabel {
			id: lblMain
			text: _control.labelText[_control.type]
			fontColor: _control.labelColor
			visible: _control.showLabel

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
			width: AppSettings.itemSmallHeight
			height: width
			visible: _control.showButtons && _control.type === SetInputField.Type.TimeType
			enabled: _control.editable

			anchors {
				left: _control.showLabel ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				txtMain.text = AppUtils.setTypeOperation(_control.type, true, txtMain.text, false);
				_control.valueChanged(txtMain.text);
			}
		}

		TPButton {
			id: btnDecrease
			imageSource: "minus"
			hasDropShadow: false
			width: AppSettings.itemSmallHeight
			height: width
			visible: _control.showButtons
			enabled: _control.editable

			anchors {
				left: btnIncreaseMinutes.visible ? btnIncreaseMinutes.right : _control.showLabel ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				_control.clearInput = false;
				const value = txtMain.text !== "" ? txtMain.text : txtMain.origText;
				txtMain.text = AppUtils.setTypeOperation(_control.type, false, value, false);
				_control.valueChanged(txtMain.text);
			}
		}

		TPTextInput {
			id: txtMain
			heightAdjustable: false
			validator: _control.validatorType[_control.type]
			inputMethodHints: _control.type <= SetInputField.Type.RepType ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly
			maximumLength: _control.maxLen[_control.type]
			readOnly: !_control.editable
			padding: 0
			focus: true

			property string origText

			width: {
				switch (_control.type) {
				case SetInputField.WeightType:
					return Math.min(_control.availableWidth * 0.35, 2 * height);
				case SetInputField.RepType:
					return Math.min(_control.availableWidth * 0.25, 1.5 * height);
				case SetInputField.Type.TimeType:
					return Math.min(_control.availableWidth * 0.4, 3 * height);
				case SetInputField.SetType:
					return Math.min(_control.availableWidth * 0.2, height);
				}
			}

			anchors {
				left: _control.showButtons ? btnDecrease.right : _control.showLabel ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onEnterOrReturnKeyPressed: {
				_control.clearInput = true;
				_control.enterOrReturnKeyPressed();
			}

			onActiveFocusChanged: {
				if (activeFocus) {
					if (_control.clearInput && _control.editable) {
						origText = text;
						txtMain.clear();
						//In case the window loose focus, when returning do not erase what was being written before the loosing of focus
						_control.clearInput = false;
					}
				}
				else {
					if (text === "")
						text = origText;
				}
			}

			onTextEdited: {
				if (_control.type === SetInputField.Type.TimeType)
					text = _control.formatTime(text);
				else
					text = _control.sanitizeText(text);
				_control.valueChanged(text);
			}
			onTextChanged: if (!activeFocus) _control.clearInput = true;
		} //TextInput

		TPButton {
			id: btnIncrease
			imageSource: "plus"
			hasDropShadow: false
			width: AppSettings.itemSmallHeight
			height: width
			visible: _control.showButtons
			enabled: _control.editable

			anchors {
				left: txtMain.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				_control.clearInput = false;
				const value = txtMain.text !== "" ? txtMain.text : txtMain.origText;
				txtMain.text = AppUtils.setTypeOperation(_control.type, true, value, true);
				_control.valueChanged(txtMain.text);
			}
		}

		TPButton {
			id: btnDecreaseSeconds
			imageSource: "minus"
			hasDropShadow: false
			width: AppSettings.itemSmallHeight
			height: width
			visible: _control.showButtons && _control.type === SetInputField.Type.TimeType
			enabled: _control.editable

			anchors {
				left: btnIncrease.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				txtMain.text = AppUtils.setTypeOperation(_control.type, false, txtMain.text, true);
				_control.valueChanged(txtMain.text);
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
		switch (digits.length) {
		case 0: return ""; // Empty input shows nothing
		case 1: return digits;
		case 2:
			if (digits === txtMain.text)
				return digits + ":";
			else
				return digits;
		default: return digits.substring(0, 2) + ":" + digits.substring(2);
		}
	}
} //FocusScope
