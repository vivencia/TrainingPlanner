import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

ColumnLayout {
	id: _control
	spacing: 10

//public:
	property bool bReady: false
	signal netConfigurationResult(bool success);

//private:
	property string _message

	Connections {
		target: AppUserModel

		function onUserImportFromServerStatus(can_import: bool, is_imported: bool, message: string): void {
			_control.bReady = is_imported;
			btnCheckEMail._can_click = !can_import && !is_imported;
			_message = message;
		}
	}

	TPRadioButtonOrCheckBox {
		id: optNewUser
		text: AppUserModel.newUserLabel
		multiLine: true
		Layout.fillWidth: true

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
		label: AppUserModel.passwordLabel
		enabled: txtEmail.inputOK
		Layout.fillWidth: true
	}

	TPButton {
		id: btnCheckEMail
		text: AppUserModel.checkEmailLabel
		enabled: _can_click && txtEmail.inputOK && passwordControl.passwordOK
		autoSize: true
		Layout.alignment: Qt.AlignCenter

		property bool _can_click: true

		onClicked: {
			AppUserModel.checkExistingAccount(txtEmail.text.trim(), passwordControl.getPassword());
			_can_click = false;
		}
	}

	TPLabel {
		text: _control._message
		visible: _control._message.length > 0
		singleLine: false
		useBackground: true
		Layout.fillWidth: true
	}
}
