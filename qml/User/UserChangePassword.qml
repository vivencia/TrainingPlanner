import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
	id: _passwdDlg
	keepAbove: true
	dim: true
	width: AppSettings.pageWidth * 0.8

	onOpened: {
		txtCurPassword.setPasswordText("");
		txtNewPassword.setPasswordText("");
		txtConfirmPassword.setPasswordText("");
		txtCurPassword.forceActiveFocus();
	}

	TPLabel {
		id: lblTitle
		text: qsTr("Change password")
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 15
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
			topMargin: 10
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
			top: txtCurPassword.bottom
			topMargin: 0
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
			topMargin: 0
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
			autoSize: true
			enabled: false
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				AppUserModel.changePassword(txtCurPassword.getPassword(), txtNewPassword.getPassword());
				_passwdDlg.closePopup();
				enabled = false;
			}
		}

		TPButton {
			id: btn2
			text: qsTr("Cancel")
			autoSize: true
			Layout.alignment: Qt.AlignCenter
			Layout.maximumWidth: _passwdDlg.availableWidth - btn1.width - 10

			onClicked: _passwdDlg.closePopup();
		}
	}

	function show(ypos: int): void {
		const totalHeight = lblTitle.height + txtCurPassword.height + txtNewPassword.height + txtConfirmPassword.height;
		_passwdDlg.height = btn1.height + totalHeight + 30;
		_passwdDlg.showInWindow(ypos);
	}
}
