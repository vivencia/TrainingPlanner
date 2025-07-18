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
				mainwindow.displayResultMessage(qsTr("User already registered"), qsTr("Click on Import button to use to import the registered profile"));
			else
				mainwindow.displayResultMessage(qsTr("User not registered"), qsTr("EMail has not been registered before or the password is wrong"));
		}

		function onUserOnlineImportFinished(result: bool): void {
			bReady = result;
			if (result)
				mainwindow.displayResultMessage(qsTr("User info imported"), qsTr("Click on Next to continue"));
			else
				mainwindow.displayResultMessage(qsTr("User info not imported"), qsTr("Could not retrieve the data from the internet"));
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
		}
	}

	TPPassword {
		id: passwordControl
		enabled: txtEmail.inputOK
		Layout.fillWidth: true
		onPasswordUnacceptable: btnCheckEMail.enabled = false;
		onPasswordAccepted: btnCheckEMail.enabled = true;
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
				userModel.checkUserOnline(txtEmail.text.trim(), passwordControl.getPassword());
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
