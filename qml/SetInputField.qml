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
	property int fontPixelSize: AppSettings.fontSizeText
	property color borderColor: "darkblue"
	property color labelColor: "black"
	property color inputColor: "white"
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
		bottom: 0.0
		top: 99.9;
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
			font.pixelSize: fontPixelSize
			color: labelColor

			anchors {
				left: parent.left
				leftMargin: 2
				verticalCenter: parent.verticalCenter
			}
		}

		RoundButton {
			id: btnIncreaseMinutes
			padding: 0
			spacing: 2
			width: 25
			height: 25
			visible: type === SetInputField.Type.TimeType

			anchors {
				left: lblMain.visible ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			Image {
				source: "qrc:/images/"+darkIconFolder+"plus.png"
				anchors.fill: parent
			}

			onClicked: {
				valueChanged(runCmd.addTimeToStrTime(txtMain.text, 1, 0));
			}
		}

		RoundButton {
			id: btnDecrease
			padding: 0
			spacing: 2
			width: 25
			height: 25

			anchors {
				left: btnIncreaseMinutes.visible ? btnIncreaseMinutes.right : lblMain.visible ? lblMain.right : parent.left
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			Image {
				source: "qrc:/images/"+darkIconFolder+"minus.png"
				anchors.fill: parent
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
							nbr -= 2.5
					break;
					case SetInputField.Type.RepType:
						if (str === "")
							nbr = 5;
						else
							nbr = str*1;
						if (str.indexOf('.') === -1)
							nbr -= 1;
						else
							nbr -= 0.5
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
			fontPSize: fontPixelSize
			inputMethodHints: type <= SetInputField.Type.RepType ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly
			maximumLength: maxLen[type]
			readOnly: type === SetInputField.Type.TimeType
			width: type === SetInputField.Type.TimeType ? 50 : type === SetInputField.Type.WeightType ? 35 : 30
			padding: 0
			focus: type !== SetInputField.Type.TimeType
			//focus: true
			textColor: inputColor

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
					if (!acceptableInput)
						text = origText;
				}
			}

			onEditingFinished: {
				if (acceptableInput)
					valueChanged(sanitizeText(text));
			}

			MouseArea {
				id: mousearea
				anchors.fill: parent
				enabled: type === SetInputField.Type.TimeType

				onClicked: openTimerDialog();
			}
		} //TextInput

		RoundButton {
			id: btnIncrease
			padding: 0
			spacing: 2
			width: 25
			height: 25

			anchors {
				left: txtMain.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			Image {
				source: "qrc:/images/"+darkIconFolder+"plus.png"
				anchors.fill: parent
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
						else
							nbr += 2.5
						if (nbr > 999.99)
							return;
					break;
					case SetInputField.Type.RepType:
						nbr = str*1;
						if (str.indexOf('.') === -1)
							nbr += 1;
						else
							nbr += 0.5
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

		RoundButton {
			id: btnDecreaseSeconds
			padding: 0
			spacing: 2
			width: 20
			height: 20
			visible: type === SetInputField.Type.TimeType

			anchors {
				left: btnIncrease.right
				leftMargin: 1
				rightMargin: 1
				verticalCenter: parent.verticalCenter
			}

			Image {
				source: "qrc:/images/"+darkIconFolder+"minus.png"
				anchors.fill: parent
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
