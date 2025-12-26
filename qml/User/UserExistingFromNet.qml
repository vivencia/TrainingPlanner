import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"
import "../Pages"

ColumnLayout {
	id: importModule

	property bool bReady: false
	property bool bImport: false

	Connections {
		target: userModel
		function onUserOnlineCheckResult(registered: bool): void {
			bImport = registered;
			if (registered)
				mainwindow.displayResultMessage(qsTr("Existing user account found"), Qt.platform.os !== "android" ?
					qsTr("You can click on the Import button to download all the data for the user") :
					qsTr("You can tap on the Import button to download all the data for the user"), "", 8000);
			else
				mainwindow.displayResultMessage(qsTr("User account not found"),
					qsTr("E-mail has not been registered before or the password is wrong"), "", 5000);
		}

		function onUserOnlineImportFinished(result: bool): void {
			bReady = result;
			if (result) {
				mainwindow.displayResultMessage(qsTr("User configuration imported"), Qt.platform.os !== "android" ?
					qsTr("Click on Next to start using the app") :
					qsTr("Tap on Next to start using the app"), "", 10000);
				mainwindow.firstTimeDlg.nextStartsTheApp = true;
			}
			else
				mainwindow.displayResultMessage(qsTr("User data not imported"),
					qsTr("Could not retrieve the data from the server"), "", 5000);
		}
	}

	Component.onCompleted: spacing = (Qt.platform.os !== "android") ? 10 : 0

	TPRadioButtonOrCheckBox {
		id: optNewUser
		text: userModel.newUserLabel
		multiLine: true
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 0

		onClicked: {
			bReady = checked;
			optImportUser.checked = !checked;
			if (checked)
				userModel.createMainUser();
		}
	}

	TPRadioButtonOrCheckBox {
		id: optImportUser
		text: userModel.existingUserLabel
		multiLine: true
		enabled: userModel.canConnectToServer
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
		text: userModel.emailLabel
		enabled: optImportUser.checked
		Layout.fillWidth: true

		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 0
	}

	TPTextInput {
		id: txtEmail
		enabled: optImportUser.checked
		heightAdjustable: false
		inputMethodHints: Qt.ImhLowercaseOnly|Qt.ImhEmailCharactersOnly|Qt.ImhNoAutoUppercase
		ToolTip.text: userModel.invalidEmailLabel
		Layout.fillWidth: true

		property bool inputOK: false

		onEnterOrReturnKeyPressed: {
			if (inputOK)
				txtPassword.forceActiveFocus();
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
			text: userModel.checkEmailLabel
			enabled: false
			autoSize: true

			onClicked: {
				userModel.checkExistingAccount(txtEmail.text.trim(), passwordControl.getPassword());
				enabled = false;
			}
		}

		TPButton {
			id: btnImport
			text: userModel.importUserLabel
			enabled: bImport
			autoSize: true

			onClicked: {
				userModel.importFromOnlineServer();
				bImport = false;
			}
		}
	}
}
