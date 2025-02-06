import QtQuick
import QtQuick.Controls

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

	required property TPPage parentPage
	required property int userRow
	property bool bReady: false
	property bool bImport: false
	readonly property int nControls: 5
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
		text: qsTr("Create a new user")
		multiLine: true
		height: itemHeight

		onCheckedChanged: bReady === checked;
	}

	TPRadioButton {
		id: optImportUser
		text: qsTr("User already registered")
		multiLine: true
		height: itemHeight
	}

	TPLabel {
		id: lblEmail
		text: userModel.emailLabel
		enabled: optImportUser.checked

		anchors {
			top: optImportUser.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtEmail
		text: userModel.email(userRow)
		enabled: optImportUser.checked
		ToolTip.text: qsTr("Invalid email address")
		height: controlsHeight
		width: parent.width*0.9

		onTextEdited: {
			if (text.length === 0 || (text.indexOf("@") !== -1 && text.indexOf(".") !== -1)) {
				ToolTip.visible = false;
			}
			else
				ToolTip.visible = true;
		}

		anchors {
			top: lblEmail.bottom
			left: parent.left
			leftMargin: 5
		}
	}

	Row {
		spacing: 0
		padding: 0
		height: controlsHeight
		enabled: optImportUser.checked

		anchors {
			top: txtEmail.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
		}

		TPButton {
			id: btnCheckEMail
			text: qsTr("Check")

			onClicked: userModel.checkUserOnline(txtEmail.text);
		}

		TPButton {
			id: btnImport
			text: qsTr("Import")
			enabled: bImport

			onClicked: userModel.importFromOnlineServer();
		}
	}
}
