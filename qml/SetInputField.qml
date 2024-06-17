import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
	required property int type
	required property int availableWidth

	property bool showLabel: true
	property alias text: txtMain.text
	property string windowTitle
	property var alternativeLabels: []

	property bool bClearInput: true
	property color borderColor: AppSettings.fontColor
	property color labelColor: "black"
	property color inputColor: AppSettings.fontColor
	property color backColor: "white"

	signal valueChanged(string str)
	signal enterOrReturnKeyPressed()

	implicitWidth: availableWidth
	implicitHeight: 40

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
		locale: AppSettings.appLocale
	}

	DoubleValidator {
		id: val_rep
		bottom: 0.00
		top: 99.99;
		decimals: 2
		notation: DoubleValidator.StandardNotation
		locale: AppSettings.appLocale
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

	property var validatorType: [val_weigth, val_rep, val_time, val_set]
	property var maxLen: [5,4,5,1]
	property var labelText: [ qsTr("Weight") + AppSettings.weightUnit + ':', qsTr("Reps:"), qsTr("Rest time:"), qsTr("SubSets:") ]
	property var origText

	Rectangle {
		anchors.fill: parent
		border.color: borderColor
		radius: 6
		color: backColor

		Label {
			id: lblMain
			text: alternativeLabels.length === 0 ? labelText[type] : alternativeLabels[type];
			padding: 0
			visible: showLabel
			font.bold: true
			font.pointSize: AppSettings.fontSizeText
			color: enabled ? labelColor : "gray"
			anchors {
				left: parent.left
				leftMargin: 2
				verticalCenter: parent.verticalCenter
			}
		}

		TPRoundButton {
			id: btnIncreaseMinutes
			padding: 0
			spacing: 2
			width: 25
			height: 25
			visible: type === SetInputField.Type.TimeType
			imageName: "plus.png"

			anchors {
				left: lblMain.visible ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				txtMain.text = runCmd.addTimeToStrTime(txtMain.text, 1, 0)
				valueChanged(txtMain.text);
			}
		}

		TPRoundButton {
			id: btnDecrease
			padding: 0
			spacing: 2
			width: 25
			height: 25
			imageName: "minus.png"

			anchors {
				left: btnIncreaseMinutes.visible ? btnIncreaseMinutes.right : lblMain.visible ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				bClearInput = false;
				txtMain.text = runCmd.setTypeOperation(type, false, txtMain.text !== "" ? txtMain.text : origText)
				valueChanged(txtMain.text);
			}
		}

		TPTextInput {
			id: txtMain
			validator: validatorType[type]
			inputMethodHints: type <= SetInputField.Type.RepType ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly
			maximumLength: maxLen[type]
			readOnly: type === SetInputField.Type.TimeType
			width: type === SetInputField.Type.TimeType ? 50 : type === SetInputField.Type.WeightType ? 45 : 35
			padding: 0
			focus: type !== SetInputField.Type.TimeType

			anchors {
				left: btnDecrease.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			Keys.onReturnPressed: { //Alphanumeric keyboard
				bClearInput = true;
				enterOrReturnKeyPressed();
			}
			Keys.onEnterPressed: { //Numeric keyboard
				bClearInput = true;
				enterOrReturnKeyPressed();
			}

			onActiveFocusChanged: {
				if (activeFocus) {
					if (type === SetInputField.Type.TimeType) {
						openTimerDialog();
						return;
					}
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

		TPRoundButton {
			id: btnIncrease
			padding: 0
			spacing: 2
			width: 25
			height: 25
			imageName: "plus.png"

			anchors {
				left: txtMain.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				bClearInput = false;
				txtMain.text = runCmd.setTypeOperation(type, true, txtMain.text !== "" ? txtMain.text : origText)
				valueChanged(txtMain.text);
			}
		}

		TPRoundButton {
			id: btnDecreaseSeconds
			padding: 0
			spacing: 2
			width: 20
			height: 20
			visible: type === SetInputField.Type.TimeType
			imageName: "minus.png"

			anchors {
				left: btnIncrease.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				const secs = parseInt(txtMain.text.substring(3, 5));
				const nbr = secs > 5 ? -5 : -1;
				txtMain.text = runCmd.addTimeToStrTime(txtMain.text, 0, nbr);
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

	function timeChanged(strTime) {
		txtMain.text = strTime;
		valueChanged(strTime);
		enterOrReturnKeyPressed();
	}

	function openTimerDialog() {
		requestTimer (this, qsTr("Time of rest until ") + windowTitle, txtMain.text.substring(0, 2), txtMain.text.substring(3, 5));
	}
} //FocusScope
