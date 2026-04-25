import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
	id: _passwdDlg
	keepAbove: true
	dim: true
	width: AppSettings.pageWidth * 0.8
	height: mainLayout.childrenRect.height * 1.1 + titleBar.height

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
			fill: _passwdDlg.titleBar
			leftMargin: _passwdDlg.btnClose.width
			rightMargin: _passwdDlg.btnClose.width
		}
	}

	ColumnLayout {
		id: mainLayout
		spacing: 5

		anchors {
			top: _passwdDlg.titleBar.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		TPPassword {
			id: txtCurPassword
			customLabel: qsTr("Current password: ")
			showAcceptButton: false
			Layout.fillWidth: true

			onPasswordAccepted: {
				txtNewPassword.enabled = true;
				txtNewPassword.forceActiveFocus();
			}
			onPasswordUnacceptable: txtNewPassword.enabled = false;
		}

		TPPassword {
			id: txtNewPassword
			enabled: false
			customLabel: qsTr("New password: ")
			showAcceptButton: false
			Layout.fillWidth: true

			onPasswordAccepted: {
				txtConfirmPassword.enabled = true;
				txtConfirmPassword.forceActiveFocus();
			}
			onPasswordUnacceptable: txtConfirmPassword.enabled = false;
		}

		TPPassword {
			id: txtConfirmPassword
			enabled: false
			customLabel: qsTr("Confirm new password: ")
			matchAgainst: txtNewPassword.getPassword();
			showAcceptButton: false
			Layout.fillWidth: true

			onPasswordAccepted: btn1.enabled = true;
			onPasswordUnacceptable: btn1.enabled = false;
		}

		RowLayout {
			spacing: 0
			Layout.fillWidth: true

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
	}//ColumnLayout
}//TPPopup
