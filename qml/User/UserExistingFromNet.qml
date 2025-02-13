import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"
import "../Pages"

Frame {
	id: frmImport
	spacing: 15
	padding: 0
	height: moduleHeight
	implicitHeight: Math.min(height, moduleHeight)

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	property bool bReady: false
	property bool bImport: false
	readonly property int nControls: 7
	readonly property int controlsHeight: 25
	readonly property int moduleHeight: nControls*(controlsHeight) + 15

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

	TPRadioButton {
		id: optNewUser
		text: userModel.newUserLabel
		multiLine: true

		onClicked: {
			bReady = checked;
			optImportUser.checked = !checked;
			if (checked)
				userModel.createMainUser();
		}

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPRadioButton {
		id: optImportUser
		text: userModel.existingUserLabel
		multiLine: true

		onClicked: {
			optNewUser.checked = !checked;
			if (checked)
				txtEmail.forceActiveFocus();
		}

		anchors {
			top: optNewUser.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPLabel {
		id: lblEmail
		text: userModel.emailLabel
		enabled: optImportUser.checked

		anchors {
			top: optImportUser.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtEmail
		enabled: optImportUser.checked
		ToolTip.text: userModel.invalidEmailLabel
		height: controlsHeight

		property bool inputOK: false

		onEnterOrReturnKeyPressed: {
			if (inputOK)
				txtPassword.forceActiveFocus();
		}

		onTextEdited: {
			inputOK = (text.length === 0 || (text.indexOf("@") !== -1 && text.indexOf(".") !== -1));
			ToolTip.visible = !inputOK;
		}

		anchors {
			top: lblEmail.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPPassword {
		id: passwordControl
		enabled: txtEmail.inputOK

		anchors {
			top: txtEmail.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onPasswordUnacceptable: btnCheckEMail.enabled = false;
		onPasswordAccepted: btnCheckEMail.enabled = true;
	}

	RowLayout {
		spacing: 10
		height: controlsHeight

		anchors {
			top: passwordControl.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		TPButton {
			id: btnCheckEMail
			text: userModel.checkEmailLabel
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				userModel.checkUserOnline(txtEmail.text.trim(), passwordControl.getPassword());
				enabled = false;
			}
		}

		TPButton {
			id: btnImport
			text: userModel.importUserLabel
			enabled: bImport
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				userModel.importFromOnlineServer();
				bImport = false;
			}
		}
	}
}
