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
		bottom: 0.0
		top: 999.9;
		decimals: 1
		locale: AppSettings.appLocale
	}

	DoubleValidator {
		id: val_rep
		bottom: 0.00
		top: 99.99;
		decimals: 1
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
			color: labelColor

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

			onClicked: valueChanged(runCmd.addTimeToStrTime(txtMain.text, 1, 0));
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
				var nbr;
				var str = sanitizeText(txtMain.text);

				switch (type) {
					case SetInputField.Type.WeightType:
						if (str === "")
							nbr = 5;
						else
							nbr = str*1;

						if (str.indexOf('.') === -1) {
							if (nbr <= 40) {
								nbr -= 2;
								if (nbr % 2 !== 0)
									nbr--;
							}
							else
								nbr -= 5;
						}
						else
						{
							if (str.endsWith('5'))
								nbr -= 2.5
							else
								nbr -= 5;
						}
					break;
					case SetInputField.Type.RepType:
						if (str === "")
							nbr = 5;
						else
							nbr = str*1;
						if (str.indexOf('.') === -1)
							nbr -= 1;
						else
						{
							if (str.endsWith('5'))
								nbr -= 0.5
							else
								nbr -= 1;
						}
					break;
					case SetInputField.Type.SetType:
						if (str === "")
							nbr = 1;
						else
							nbr = parseInt(str);
						if (nbr < 1)
							return;
						nbr--;
					break;
					case SetInputField.Type.TimeType:
						valueChanged(runCmd.addTimeToStrTime(txtMain.text, -1, 0));
					return;
				}
				if (nbr < 0)
					return;
				bClearInput = false;
				valueChanged(nbr.toString());
			}
		}

		TPTextInput {
			id: txtMain
			validator: validatorType[type]
			inputMethodHints: type <= SetInputField.Type.RepType ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly
			maximumLength: maxLen[type]
			readOnly: type === SetInputField.Type.TimeType
			width: type === SetInputField.Type.TimeType ? 50 : type === SetInputField.Type.WeightType ? 40 : 35
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
				if(activeFocus) {
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
					valueChanged(sanitizeText(text));
				}
			}

			onEditingFinished: {
				//if (acceptableInput)
					valueChanged(sanitizeText(text));
			}

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
				var nbr;
				var str = sanitizeText(txtMain.text);

				switch (type) {
					case SetInputField.Type.WeightType:
						nbr = str*1;
						if (str.indexOf('.') === -1) {
							if (nbr <= 40) {
								nbr += 2;
								if (nbr % 2 !== 0)
									nbr++;
							}
							else
								nbr += 5;
						}
						else {
							if (str.endsWith('5'))
								nbr += 2.5
							else
								nbr += 5
						}
						if (nbr > 999.99)
							return;
					break;
					case SetInputField.Type.RepType:
						nbr = str*1;
						if (str.indexOf('.') === -1)
							nbr += 1;
						else
						{
							if (str.endsWith('5'))
								nbr += 0.5
							else
								nbr += 1;
						}
						if (nbr > 99.99)
							return;
					break;
					case SetInputField.Type.SetType:
						nbr = parseInt(str);
						if (nbr >= 9)
							return;
						nbr++;
					break;
					case SetInputField.Type.TimeType:
						const secs = parseInt(str.substring(3, 5));
						nbr = secs < 55 ? 5 : 1;
						valueChanged(runCmd.addTimeToStrTime(txtMain.text, 0, nbr));
					return;
				}
				bClearInput = false;
				valueChanged(nbr.toString());
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
				valueChanged(runCmd.addTimeToStrTime(txtMain.text, 0, nbr));
			}
		}
	} //Rectangle

	function sanitizeText(text) {
		if (text.indexOf(',') !== -1)
			text = text.replace(',', '.');
		if (text.indexOf('-') !== -1)
			text = text.replace('-', '');
		if (text.indexOf('E') !== -1)
			text = text.replace('E', '');
		return text.trim();
	}

	function timeChanged(strTime) {
		valueChanged(strTime);
		enterOrReturnKeyPressed();
	}

	function openTimerDialog() {
		requestTimer (this, qsTr("Time of rest until ") + windowTitle, txtMain.text.substring(0, 2), txtMain.text.substring(3, 5));
	}
} //FocusScope
