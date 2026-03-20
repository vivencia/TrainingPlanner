import QtQuick
import QtQuick.Controls

import TpQml

FocusScope {
	id: _control
	height: 2 * AppSettings.itemDefaultHeight
	implicitHeight: height

	property string customLabel: ""
	property string matchAgainst: ""
	property bool includeNotAllowableChars: true
	property bool showAcceptButton: true
	property bool passwordOK: false

	readonly property string notAllowableChars: "# &?=\'\""

	signal passwordAccepted();
	signal passwordUnacceptable();

	TPLabel {
		id: lblPassword
		text: (_control.customLabel.length === 0 ? AppUserModel.passwordLabel : _control.customLabel) +
							(_control.includeNotAllowableChars ? "(" + _control.notAllowableChars + qsTr(" not allowed)") : "")
		singleLine: true
		height: AppSettings.itemDefaultHeight

		anchors {
			top: parent.top
			topMargin: -5
			left: parent.left
			right: parent.right
		}
	}

	TPPasswordInput {
		id: txtPassword
		validator: RegularExpressionValidator { regularExpression: /^[^# &?="']*$/ }

		property bool inputOK: false
		property bool matchOK: true

		onEnterOrReturnKeyPressed: {
			_control.passwordOK = inputOK && matchOK;
			if (_control.passwordOK)
				_control.passwordAccepted();
		}

		onTextChanged: {
			_control.passwordOK = matchOK = inputOK = text.length >= 6;
			ToolTip.visible = !inputOK;
		}

		onTextEdited: {
			if (acceptableInput) {
				if (text.length < 6) {
					matchOK = inputOK = false;
					_control.passwordOK = false;
					_control.passwordUnacceptable();
					ToolTip.text = AppUserModel.invalidPasswordLabel
					ToolTip.visible = true;
				}
				else {
					inputOK = true;
					ToolTip.visible = false;
					if (_control.matchAgainst.length > 0) {
						matchOK = text === _control.matchAgainst;
						if (matchOK) {
							_control.passwordOK = true;
							_control.passwordAccepted();
						}
						else {
							_control.passwordOK = false;
							_control.passwordUnacceptable();
							ToolTip.text = qsTr("Passwords do not match")
							ToolTip.visible = true;
						}
					}
					else {
						_control.passwordOK = true;
						_control.passwordAccepted();
					}
				}
			}
		}

		anchors {
			top: lblPassword.bottom
			topMargin: 5
			left: parent.left
			right: parent.right
			rightMargin: _control.showAcceptButton ? AppSettings.itemDefaultHeight + 5 : 5
		}
	}

	TPButton {
		id: btnAccept
		imageSource: "set-completed"
		width: AppSettings.itemDefaultHeight
		height: width
		enabled: txtPassword.inputOK && txtPassword.matchOK
		visible: _control.showAcceptButton

		anchors {
			verticalCenter: txtPassword.verticalCenter
			left: txtPassword.right
		}

		onClicked: _control.passwordAccepted();
	}

	function setPasswordText(passwd: string): void {
		if (passwd.length >= 6 || passwd.length === 0)
			txtPassword.text = passwd;
	}

	function getPassword(): string {
		return txtPassword.inputOK ? txtPassword.text.trim() : "";
	}
}
