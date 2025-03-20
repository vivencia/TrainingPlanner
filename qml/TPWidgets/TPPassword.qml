import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

FocusScope {
	id: control
	height: 60
	implicitHeight: 60

	signal passwordAccepted();
	signal passwordUnacceptable();

	readonly property string notAllowableChars: "# &?=\'\""
	readonly property string customLabel: ""
	readonly property string matchAgainst: ""
	readonly property bool includeNotAllowableChars: true
	readonly property bool showAcceptButton: true

	TPLabel {
		id: lblPassword
		text: (customLabel.length === 0 ? userModel.passwordLabel : customLabel) +
					(includeNotAllowableChars ? "(" + notAllowableChars + qsTr(" not allowed)") : "")
		height: 25

		anchors {
			top: control.top
			left: parent.left
			right: parent.right
		}
	}

	TPTextInput {
		id: txtPassword
		heightAdjustable: false
		echoMode: btnShowHidePassword.show ? TextInput.Normal : TextInput.Password
		inputMethodHints: Qt.ImhSensitiveData|Qt.ImhNoPredictiveText
		validator: RegularExpressionValidator { regularExpression: /^[^# &?="']*$/ }
		ToolTip.text: matchOK ? userModel.invalidPasswordLabel : qsTr("Passwords do not match")
		focus: true
		height: 25

		property bool inputOK: false
		property bool matchOK: true

		onEnterOrReturnKeyPressed: {
			if (inputOK && matchOK)
				passwordAccepted();
		}

		onTextChanged: {
			inputOK = text.length >= 6;
			ToolTip.visible = !inputOK;
		}

		onTextEdited: {
			if (acceptableInput) {
				if (text.length < 6) {
					if (inputOK)
						passwordUnacceptable();
				}
				inputOK = text.length >= 6;
				ToolTip.visible = !inputOK;
				if (inputOK) {
					if (matchAgainst.length > 0) {
						matchOK = text === matchAgainst;
						ToolTip.visible = !matchOK;
						if (matchOK)
							passwordAccepted();
						else
							passwordUnacceptable();
					}
				}
			}
		}

		anchors {
			top: lblPassword.bottom
			topMargin: 5
			left: parent.left
			right: btnAccept.left
		}

		TPButton {
			id: btnShowHidePassword
			imageSource: show ? "hide-password.png" : "show-password.png"
			hasDropShadow: false
			imageSize: 20
			focus: false

			property bool show: false

			anchors {
				right: btnClearText.left
				rightMargin: 5
				verticalCenter: txtPassword.verticalCenter
			}

			onClicked: {
				show = !show;
				txtPassword.forceActiveFocus();
			}
		}

		TPButton {
			id: btnClearText
			imageSource: "edit-clear"
			hasDropShadow: false
			imageSize: 20
			focus: false

			anchors {
				right: txtPassword.right
				rightMargin: 5
				verticalCenter: txtPassword.verticalCenter
			}

			onClicked: {
				txtPassword.clear();
				txtPassword.forceActiveFocus();
			}
		}
	}

	TPButton {
		id: btnAccept
		imageSource: "set-completed"
		height: 25
		enabled: txtPassword.inputOK && txtPassword.matchOK
		visible: showAcceptButton

		anchors {
			verticalCenter: txtPassword.verticalCenter
			right: parent.right
		}

		onClicked: passwordAccepted();
	}

	function setPasswordText(passwd: string): void {
		if (passwd.length >= 6)
			txtPassword.text = passwd;
	}

	function getPassword(): string {
		return txtPassword.inputOK ? txtPassword.text.trim() : "";
	}
}
