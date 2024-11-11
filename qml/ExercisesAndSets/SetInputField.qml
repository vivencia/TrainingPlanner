import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets/"

FocusScope {
	id: control
	required property int type
	required property int availableWidth

	property alias text: txtMain.text
	property bool showLabel: true
	property bool showButtons: true
	property var alternativeLabels: []
	property bool bClearInput: true
	property color borderColor: appSettings.fontColor
	property color labelColor: "black"
	property color inputColor: appSettings.fontColor
	property color backColor: "white"

	readonly property var validatorType: [val_weigth, val_rep, val_time, val_set]
	readonly property var maxLen: [5,4,5,1]
	readonly property var labelText: [ qsTr("Weight") + appSettings.weightUnit + ':', qsTr("Reps:"), qsTr("Rest time:"), qsTr("SubSets:") ]
	property string origText

	signal valueChanged(string str)
	signal enterOrReturnKeyPressed()

	implicitWidth: availableWidth
	width: availableWidth
	implicitHeight: 30

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

	IntValidator {
		id: val_time
		bottom: 0
		top: 59
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
		color: backColor

		TPLabel {
			id: lblMain
			text: alternativeLabels.length === 0 ? labelText[type] : alternativeLabels[type];
			fontColor: labelColor
			width: availableWidth*0.5
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
			width: 20
			height: 20
			visible: showButtons && type === SetInputField.Type.TimeType


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
			width: 20
			height: 20
			visible: showButtons

			anchors {
				left: btnIncreaseMinutes.visible ? btnIncreaseMinutes.right : showLabel ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				bClearInput = false;
				txtMain.text = appUtils.setTypeOperation(type, false, txtMain.text !== "" ? txtMain.text : origText)
				valueChanged(txtMain.text);
			}
		}

		TPTextInput {
			id: txtMain
			validator: validatorType[type]
			inputMethodHints: type <= SetInputField.Type.RepType ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly
			maximumLength: maxLen[type]
			readOnly: type === SetInputField.Type.TimeType
			width: type === SetInputField.Type.TimeType ? Math.min(availableWidth*0.4, 2*height) : Math.min(availableWidth*0.3, 2*height)
			padding: 0
			focus: type !== SetInputField.Type.TimeType

			anchors {
				left: showButtons ? btnDecrease.right : showLabel ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onEnterOrReturnKeyPressed: {
				bClearInput = true;
				control.enterOrReturnKeyPressed();
			}

			onActiveFocusChanged: {
				if (activeFocus) {
					if (bClearInput) {
						origText = text;
						txtMain.clear();
						bClearInput = false; //In case the window loose focus, when returning do not erase what was being written before the loosing of focus
					}
				}
				else {
					if (text === "")
						text = origText;
				}
			}

			onTextEdited: valueChanged(text = sanitizeText(text));

			MouseArea {
				id: mousearea
				anchors.fill: parent
				enabled: type === SetInputField.Type.TimeType

				onClicked: openTimerDialog();
			}
		} //TextInput

		TPButton {
			id: btnIncrease
			imageSource: "plus"
			hasDropShadow: false
			width: 20
			height: 20
			visible: showButtons

			anchors {
				left: txtMain.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				bClearInput = false;
				txtMain.text = appUtils.setTypeOperation(type, true, txtMain.text !== "" ? txtMain.text : origText)
				valueChanged(txtMain.text);
			}
		}

		TPButton {
			id: btnDecreaseSeconds
			imageSource: "minus"
			hasDropShadow: false
			width: 20
			height: 20
			visible: showButtons && type === SetInputField.Type.TimeType

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

	function sanitizeText(text) {
		text = text.replace('.', ',');
		text = text.replace('-', '');
		text = text.replace('E', '');
		return text.trim();
	}
} //FocusScope
