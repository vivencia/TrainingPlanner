import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

FocusScope {
	id: control
	height: 60
	implicitHeight: 60

	property string customLabel: ""
	property string matchAgainst: ""
	property bool includeNotAllowableChars: true
	property bool showAcceptButton: true

	readonly property string notAllowableChars: "# &?=\'\""

	signal passwordAccepted();
	signal passwordUnacceptable();

	TPLabel {
		id: lblPassword
		text: (customLabel.length === 0 ? userModel.passwordLabel : customLabel) +
					(includeNotAllowableChars ? "(" + notAllowableChars + qsTr(" not allowed)") : "")
		height: 25

		anchors {
			top: parent.top
			topMargin: -5
			left: parent.left
			right: parent.right
		}
	}

	TPTextInput {
		id: txtPassword
		heightAdjustable: false
		showClearTextButton: true
		echoMode: btnShowHidePassword.show ? TextInput.Normal : TextInput.Password
		inputMethodHints: Qt.ImhSensitiveData|Qt.ImhNoPredictiveText|Qt.ImhNoAutoUppercase
		validator: RegularExpressionValidator { regularExpression: /^[^# &?="']*$/ }
		rightPadding:  defaultPadding + btnShowHidePassword.width + 5
		focus: true

		property bool inputOK: false
		property bool matchOK: true

		onEnterOrReturnKeyPressed: {
			if (inputOK && matchOK)
				passwordAccepted();
		}

		onTextChanged: {
			matchOK = inputOK = text.length >= 6;
			ToolTip.visible = !inputOK;
		}

		onTextEdited: {
			if (acceptableInput) {
				if (text.length < 6) {
					matchOK = inputOK = false;
					passwordUnacceptable();
					ToolTip.text = userModel.invalidPasswordLabel
					ToolTip.visible = true;
				}
				else {
					inputOK = true;
					ToolTip.visible = false;
					if (matchAgainst.length > 0) {
						matchOK = text === matchAgainst;
						if (matchOK)
							passwordAccepted();
						else {
							passwordUnacceptable();
							ToolTip.text = qsTr("Passwords do not match")
							ToolTip.visible = true;
						}
					}
					else
						passwordAccepted();
				}
			}
		}

		anchors {
			top: lblPassword.bottom
			topMargin: 5
			left: parent.left
			right: parent.right
			rightMargin: showAcceptButton ? appSettings.itemDefaultHeight + 5 : 5
		}

		TPButton {
			id: btnShowHidePassword
			imageSource: show ? "hide-password.png" : "show-password.png"
			hasDropShadow: false
			width: appSettings.itemDefaultHeight
			height: width
			focus: false

			property bool show: false

			anchors {
				right: parent.right
				rightMargin: txtPassword.defaultPadding + 5
				verticalCenter: txtPassword.verticalCenter
			}

			onClicked: {
				show = !show;
				txtPassword.forceActiveFocus();
			}
		}
	}

	TPButton {
		id: btnAccept
		imageSource: "set-completed"
		width: appSettings.itemDefaultHeight
		height: width
		enabled: txtPassword.inputOK && txtPassword.matchOK
		visible: showAcceptButton

		anchors {
			verticalCenter: txtPassword.verticalCenter
			left: txtPassword.right
		}

		onClicked: passwordAccepted();
	}

	function setPasswordText(passwd: string): void {
		if (passwd.length >= 6 || passwd.length === 0)
			txtPassword.text = passwd;
	}

	function getPassword(): string {
		return txtPassword.inputOK ? txtPassword.text.trim() : "";
	}
}
