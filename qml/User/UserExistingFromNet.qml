import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

ColumnLayout {
	id: _control

	property bool bReady: false
	property bool bImport: false

	signal netConfigurationResult(bool success);

	Connections {
		target: AppUserModel
		function onUserOnlineCheckResult(registered: bool): void {
			_control.bImport = registered;
			if (registered)
				ItemManager.displayMessageOnAppWindow(qsTr("Existing user account found"), Qt.platform.os !== "android" ?
					qsTr("You can click on the Import button to download all the data for the user") :
					qsTr("You can tap on the Import button to download all the data for the user"), "", 8000);
			else
				ItemManager.displayMessageOnAppWindow(qsTr("User account not found"),
												qsTr("E-mail has not been registered before or the password is wrong"), "", 5000);
		}

		function onUserOnlineImportFinished(result: bool): void {
			_control.bReady = result;
			_control.netConfigurationResult(result);
			if (result) {
				ItemManager.displayMessageOnAppWindow(qsTr("User configuration imported"), Qt.platform.os !== "android" ?
																			qsTr("Click on Next to start using the app") :
																			qsTr("Tap on Next to start using the app"), "", 10000);
			}
			else
				ItemManager.displayMessageOnAppWindow(qsTr("User data not imported"),
																		qsTr("Could not retrieve the data from the server"),"", 5000);
		}
	}

	Component.onCompleted: spacing = (Qt.platform.os !== "android") ? 10 : 0

	TPRadioButtonOrCheckBox {
		id: optNewUser
		text: AppUserModel.newUserLabel
		multiLine: true
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 0

		onClicked: {
			_control.bReady = checked;
			optImportUser.checked = !checked;
			if (checked)
				AppUserModel.createMainUser();
		}
	}

	TPRadioButtonOrCheckBox {
		id: optImportUser
		text: AppUserModel.existingUserLabel
		multiLine: true
		enabled: AppUserModel.canConnectToServer
		Layout.fillWidth: true

		onClicked: {
			optNewUser.checked = !checked;
			if (checked)
				txtEmail.forceActiveFocus();
		}

		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5
	}

	TPLabel {
		id: lblEmail
		text: AppUserModel.emailLabel
		enabled: optImportUser.checked
		Layout.fillWidth: true

		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 0
	}

	TPTextInput {
		id: txtEmail
		enabled: optImportUser.checked
		heightAdjustable: false
		inputMethodHints: Qt.ImhLowercaseOnly|Qt.ImhEmailCharactersOnly|Qt.ImhNoAutoUppercase
		ToolTip.text: AppUserModel.invalidEmailLabel
		Layout.fillWidth: true

		property bool inputOK: false

		onEnterOrReturnKeyPressed: {
			if (inputOK)
				passwordControl.forceActiveFocus();
		}

		onTextEdited: {
			inputOK = (text.length === 0 || (text.indexOf("@") !== -1 && text.indexOf(".") !== -1));
			ToolTip.visible = !inputOK;
			btnCheckEMail.enabled = passwordControl.passwordOK && inputOK;
		}
	}

	TPPassword {
		id: passwordControl
		enabled: txtEmail.inputOK
		Layout.fillWidth: true
		onPasswordUnacceptable: btnCheckEMail.enabled = false;
		onPasswordAccepted: btnCheckEMail.enabled = txtEmail.inputOK;
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 0
	}

	RowLayout {
		Layout.alignment: Qt.AlignCenter
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5

		TPButton {
			id: btnCheckEMail
			text: AppUserModel.checkEmailLabel
			enabled: false
			autoSize: true

			onClicked: {
				AppUserModel.checkExistingAccount(txtEmail.text.trim(), passwordControl.getPassword());
				enabled = false;
			}
		}

		TPButton {
			id: btnImport
			text: AppUserModel.importUserLabel
			enabled: _control.bImport
			autoSize: true

			onClicked: {
				AppUserModel.importFromOnlineServer();
				_control.bImport = false;
			}
		}
	}
}
