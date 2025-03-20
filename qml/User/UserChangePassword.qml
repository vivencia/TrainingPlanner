import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: changePasswdDlg
	keepAbove: true
	width: appSettings.pageWidth * 0.8
	height: nControls*controlsHeight

	property int startYPosition: 0
	property int finalXPos: 0
	readonly property int nControls: 8
	readonly property int controlsHeight: 25

	onOpened: {
		txtCurPassword.clear();
		txtNewPassword.clear();
		txtNewPasswordMatch.clear();
		txtCurPassword.forceActiveFocus();
	}

	TPLabel {
		id: lblTitle
		text: qsTr("Change password")

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 20
		}
	}

	TPPassword {
		id: txtCurPassword
		customLabel: qsTr("Current password: ")
		showAcceptButton: false

		onPasswordAccepted: {
			txtNewPassword.enabled = true;
			txtNewPassword.forceActiveFocus();
		}
		onPasswordUnacceptable: txtNewPassword.enabled = false;

		anchors {
			top: lblTitle.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPPassword {
		id: txtNewPassword
		enabled: false
		customLabel: qsTr("New password: ")
		showAcceptButton: false

		onPasswordAccepted: {
			txtConfirmPassword.enabled = true;
			txtConfirmPassword.forceActiveFocus();
		}
		onPasswordUnacceptable: txtConfirmPassword.enabled = false;

		anchors {
			top: lblCurPassword.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPPassword {
		id: txtConfirmPassword
		enabled: false
		customLabel: qsTr("Confirm new password: ")
		matchAgainst: txtNewPassword.getPassword();
		showAcceptButton: false

		onPasswordAccepted: btn1.enabled = true;
		onPasswordUnacceptable: btn1.enabled = false;

		anchors {
			top: txtNewPassword.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	RowLayout {
		spacing: 0

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
			bottomMargin: 5
		}

		TPButton {
			id: btn1
			text: qsTr("Change")
			flat: false
			autoResize: true
			enabled: false
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				userModel.changePassword(txtCurPassword.getPassword(), txtNewPassword.getPassword());
				changePasswdDlg.closePopup();
				enabled = false;
			}
		}

		TPButton {
			id: btn2
			text: qsTr("Cancel")
			flat: false
			autoResize: true
			Layout.alignment: Qt.AlignCenter
			Layout.maximumWidth: availableWidth - btn1.width - 10

			onClicked: changePasswdDlg.closePopup();
		}
	}

	function show(ypos: int): void {
		show1(ypos);
	}
}
