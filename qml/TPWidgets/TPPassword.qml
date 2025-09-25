import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

FocusScope {
	id: control
	height: 2 * appSettings.itemDefaultHeight
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
		text: (customLabel.length === 0 ? userModel.passwordLabel : customLabel) +
					(includeNotAllowableChars ? "(" + notAllowableChars + qsTr(" not allowed)") : "")
		singleLine: true
		height: appSettings.itemDefaultHeight

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
			passwordOK = inputOK && matchOK;
			if (passwordOK)
				passwordAccepted();
		}

		onTextChanged: {
			passwordOK = matchOK = inputOK = text.length >= 6;
			ToolTip.visible = !inputOK;
		}

		onTextEdited: {
			if (acceptableInput) {
				if (text.length < 6) {
					matchOK = inputOK = false;
					passwordOK = false;
					passwordUnacceptable();
					ToolTip.text = userModel.invalidPasswordLabel
					ToolTip.visible = true;
				}
				else {
					inputOK = true;
					ToolTip.visible = false;
					if (matchAgainst.length > 0) {
						matchOK = text === matchAgainst;
						if (matchOK) {
							passwordOK = true;
							passwordAccepted();
						}
						else {
							passwordOK = false;
							passwordUnacceptable();
							ToolTip.text = qsTr("Passwords do not match")
							ToolTip.visible = true;
						}
					}
					else {
						passwordOK = true;
						passwordAccepted();
					}
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
