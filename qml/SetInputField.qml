import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "jsfunctions.js" as JSF

FocusScope {
	required property int type
	required property int nSetNbr
	required property int availableWidth
	property bool showLabel: true
	property alias text: txtMain.text
	property string windowTitle
	property var alternativeLabels: []
	property bool bClearInput: true
	property var timerDialog: null

	signal valueChanged(string str, real value)
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
		border.color: "darkblue"
		radius: 6

		Label {
			id: lblMain
			text: alternativeLabels.length === 0 ? labelText[type] : alternativeLabels[type];
			padding: 0
			visible: showLabel

			anchors {
				left: parent.left
				leftMargin: 2
				verticalCenter: parent.verticalCenter
			}
		}

		ToolButton {
			id: btnIncreaseTime
			padding: 0
			spacing: 2
			width: 25
			height: 25
			visible: type === SetInputField.Type.TimeType ? nSetNbr >= 1 : false

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
				var nbr = parseInt(JSF.getHourOrMinutesFromStrTime(txtMain.text));
				if (nbr < 59)
					nbr++;
				else
					nbr = 0;
				txtMain.text = JSF.intTimeToStrTime(nbr) + txtMain.text.substring(2, 5);
				changeText(txtMain.text, nbr);
			}
		}

		ToolButton {
			id: btnDecrease
			padding: 0
			spacing: 2
			width: 25
			height: 25
			visible: type === SetInputField.Type.TimeType ? nSetNbr >= 1 : true

			anchors {
				left: btnIncreaseTime.visible ? btnIncreaseTime.right : lblMain.visible ? lblMain.right : parent.left
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
							nbr = parseFloat(str);
						if (str.indexOf('.') === -1)
							nbr -= 5;
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
						nbr--;
					break;
					case SetInputField.Type.TimeType:
						nbr = parseInt(JSF.getHourOrMinutesFromStrTime(str));
						if (nbr > 0)
							nbr--;
						else
							nbr = 59;
						str = JSF.intTimeToStrTime(nbr) + str.substring(2, 5);
						txtMain.text = str;
					break;
				}
				if (nbr < 0)
					return;
				if (type !== SetInputField.Type.TimeType) {
					txtMain.text = nbr.toString();
					bClearInput = false;
				}
				changeText(txtMain.text, nbr);
			}
		}

		TextInput {
			id: txtMain
			font.bold: true
			font.pixelSize: AppSettings.fontSizeText
			validator: validatorType[type]
			inputMethodHints: type <= SetInputField.Type.RepType ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly
			maximumLength: maxLen[type]
			readOnly: type === SetInputField.Type.TimeType
			width: type === SetInputField.Type.TimeType ? 40 : 20
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
					if (bClearInput) {
						txtMain.clear();
						bClearInput = false; //In case the window loose focus, when returning do not erase what was being written before the loosing of focus
					}
				}
				else {
					if (!acceptableInput)
						text = origText;
				}
			}

			onTextEdited: {
				if (acceptableInput) {
					var nbr;
					switch (type) {
						case SetInputField.Type.WeightType:
						case SetInputField.Type.RepType:
							nbr = parseFloat(text);
						break;
						case SetInputField.Type.SetType:
						case SetInputField.Type.TimeType:
							nbr = parseInt(text);
						break;
					}
					changeText(text, nbr);
				}
			}

			MouseArea {
				anchors.fill: parent
				enabled: type === SetInputField.Type.TimeType

				onClicked: {
					if (setNbr >=1) {
						if (timerDialog === null) {
							var component = Qt.createComponent("TimerDialog.qml");
							timerDialog = component.createObject(this, { bJustMinsAndSecs:true, simpleTimer:false, windowTitle:windowTitle });
							timerDialog.onUseTime.connect(timeChanged);
						}
						timerDialog.mins = JSF.getHourOrMinutesFromStrTime(txtMain.text);
						timerDialog.secs = JSF.getMinutesOrSeconsFromStrTime(txtMain.text);
						timerDialog.open();
					}
				}
			}
		} //TextInput

		ToolButton {
			id: btnIncrease
			padding: 0
			spacing: 2
			width: 25
			height: 25
			visible: type === SetInputField.Type.TimeType ? nSetNbr >= 1 : true

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
						if (str.indexOf('.') === -1)
							nbr += 5;
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
						nbr++;
						if (nbr > 9)
							return;
					break;
					case SetInputField.Type.TimeType:
						nbr = parseInt(JSF.getMinutesOrSeconsFromStrTime(str));
						if (nbr < 55)
							nbr += 5;
						else if ( nbr < 59)
							nbr++;
						else
							nbr = 0;
						str = str.substring(0, 3) + JSF.intTimeToStrTime(nbr);
						txtMain.text = str;
					break;
				}
				if (type !== SetInputField.Type.TimeType) {
					txtMain.text = nbr.toString();
					bClearInput = false;
				}
				changeText(txtMain.text, nbr);
			}
		}

		ToolButton {
			id: btnDecreaseTime
			padding: 0
			spacing: 2
			width: 20
			height: 20
			visible: type === SetInputField.Type.TimeType ? nSetNbr >= 1 : false

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
				var nbr = parseInt(JSF.getMinutesOrSeconsFromStrTime(txtMain.text));
				if (nbr > 5)
					nbr -= 5;
				else if (nbr > 0)
					nbr--
				else
					nbr = 59;
				txtMain.text = txtMain.text.substring(0, 3) + JSF.intTimeToStrTime(nbr);
				changeText(txtMain.text, nbr);
			}
		}

		Label {
			text: nSetNbr >=1 ? qsTr("<- Leading to set") : qsTr("<- Time between exercises not computed")
			visible: type === SetInputField.Type.TimeType
			anchors {
				left: nSetNbr >= 1 ? btnDecreaseTime.right : txtMain.right
				verticalCenter: parent.verticalCenter
			}
			font.pixelSize: AppSettings.fontSizeLists
			wrapMode: Text.WordWrap
			width: availableWidth - x
		}

		Component.onDestruction: {
			if (timerDialog !== null)
				timerDialog.destroy();
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
		txtMain.text = strTime;
		changeText(strTime, 0);
		enterOrReturnKeyPressed();
	}

	function changeText(text, nbr) {
		origText = text;
		valueChanged(text, nbr);
	}

	Component.onCompleted: origText = text;
} //FocusScope
