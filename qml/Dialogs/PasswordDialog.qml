import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Dialogs
import TpQml.Widgets

TPPopup {
	id: _passwdDlg
	keepAbove: true
	dim: true
	width: AppSettings.pageWidth * 0.8
	height: titleBar.height + topRow.height + newPasswordLoader.item_height + txtPassword.item_height
																+ chkStorePassword.item_height + bottomRow.height
	useShape: true
	showBorder: true
	showTitleBar: true
	open_in_window: true
	canSlideToClose: true

//public:
	property int mode
	property string title
	property string message
	property int request_id
	property bool show_password_label: false
	property bool store_password
	property bool show_store_option

	signal passwordAcquired(bool proceed, int request_id, string passwd, bool store);
	signal passwordCreated(bool proceed, int request_id, string passwd, bool store);
	signal passwordChanged(bool proceed, int request_id, string old_passwd, string new_passwd, bool store);

//private:
	property TPPassword txtNewPassword: null
	property TPPassword txtConfirmNewPassword: null

	onOpened: {
		if (mode !== ItemManager.DM_GET_PASSWORD) {
			show_password_label = true;
			if (txtNewPassword) {
				txtNewPassword.reset();
				txtNewPassword.forceActiveFocus();
			}
			if (txtConfirmNewPassword)
				txtConfirmNewPassword.reset();
		}
		txtPassword.reset();
		txtPassword.forceActiveFocus();
	}

	onCloseActionExeced: (btn_id) => {
		if (btn_id === 0) {
			switch (mode) {
				case ItemManager.DM_GET_PASSWORD:
					passwordAcquired(true, _passwdDlg.request_id, txtPassword.getPassword(), chkStorePassword.checked);
					break;
				case ItemManager.DM_NEW_PASSWORD:
					passwordCreated(true, _passwdDlg.request_id, txtNewPassword.getPassword(), chkStorePassword.checked);
					break;
				case ItemManager.DM_CHANGE_PASSWORD:
					passwordChanged(true, _passwdDlg.request_id, txtPassword.getPassword(), txtNewPassword.getPassword(),
																							chkStorePassword.checked);
					break;
			}
		} else {
			switch (mode) {
				case ItemManager.DM_GET_PASSWORD:
					passwordAcquired(false, -1, "", false);
					break;
				case ItemManager.DM_NEW_PASSWORD:
					passwordCreated(false, -1, "", false);
					break;
				case ItemManager.DM_CHANGE_PASSWORD:
					passwordChanged(false, -1, "", "", false);
					break;
			}
		}
	}

	TPLabel {
		id: lblTitle
		text: _passwdDlg.title
		horizontalAlignment: Text.AlignHCenter
		anchors {
			top: _passwdDlg.contentItem.top
			left: _passwdDlg.contentItem.left
			right: _passwdDlg.contentItem.right
			margins: 5
			rightMargin: _passwdDlg.titleBar.titleBarButtons.width
		}
	}

	RowLayout {
		id: topRow
		anchors {
			top: lblTitle.bottom
			left: _passwdDlg.contentItem.left
			right: _passwdDlg.contentItem.right
			margins: 5
			topMargin: 10
		}

		TPImage {
			id: imgElement
			source: "password"
			Layout.preferredWidth: AppSettings.itemExtraLargeHeight
			Layout.preferredHeight: AppSettings.itemExtraLargeHeight
			Layout.alignment: Qt.AlignVCenter
		}

		TPLabel {
			id: lblMessage
			text: _passwdDlg.message
			singleLine: false
			horizontalAlignment: Text.AlignJustify
			visible: _passwdDlg.message.length > 0
			Layout.preferredWidth: _passwdDlg.width - imgElement.width - 20
		}
	}

	Loader {
		id: newPasswordLoader
		asynchronous: true
		active: _passwdDlg.mode !== ItemManager.DM_GET_PASSWORD

		readonly property int item_height: item ? item.height + 10 : 0

		anchors {
			top: topRow.bottom
			left: _passwdDlg.contentItem.left
			right: _passwdDlg.contentItem.right
			margins: 5
		}

		sourceComponent: Item {
			height: newPasswordControl.height + confirmPasswordControl.height + 8

			TPPassword {
				id: newPasswordControl
				label: qsTr("New password: ")
				showNotAllowableChars: true

				anchors {
					left: parent.left
					right: parent.right
					top: parent.top
					margins: 2
				}

				Component.onCompleted: _passwdDlg.txtNewPassword = this;
			}

			TPPassword {
				id: confirmPasswordControl
				label: qsTr("Confirm new password: ")
				matchAgainst: newPasswordControl.getPassword();
				enabled: newPasswordControl.passwordOK

				anchors {
					left: parent.left
					right: parent.right
					top: newPasswordControl.bottom
					margins: 2
				}

				Component.onCompleted: _passwdDlg.txtConfirmNewPassword = this;
			}
		}
	}

	TPPassword {
		id: txtPassword
		label: show_password_label ? (_passwdDlg.mode !== ItemManager.DM_CHANGE_PASSWORD
													? AppUserModel.passwordLabel : qsTr("Current password: ")) : ""
		visible: _passwdDlg.mode !== ItemManager.DM_NEW_PASSWORD
		enabled: _passwdDlg.txtConfirmNewPassword ? _passwdDlg.txtConfirmNewPassword.passwordOK : true

		readonly property int item_height: visible ? defaultHeight + 10 : 0

		anchors {
			top: newPasswordLoader.active ? newPasswordLoader.bottom : topRow.bottom
			left: _passwdDlg.contentItem.left
			right: _passwdDlg.contentItem.right
			margins: 5
		}

		onPasswordEntered: _passwdDlg.closePopup(0);
	}

	TPRadioButtonOrCheckBox {
		id: chkStorePassword
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		text: qsTr("Save password")
		checked: _passwdDlg.store_password
		visible: _passwdDlg.show_store_option

		readonly property int item_height: visible ? defaultHeight + 10 : 0

		anchors {
			bottom: bottomRow.top
			left: _passwdDlg.contentItem.left
			right: _passwdDlg.contentItem.right
			margins: 5
		}

		onClicked: _passwdDlg.store_password = !_passwdDlg.store_password;
	}

	Row {
		id: bottomRow
		spacing: (_passwdDlg.width - btn1.width - btn2.width) / 3

		anchors {
			bottom: _passwdDlg.contentItem.bottom
			left: _passwdDlg.contentItem.left
			right: _passwdDlg.contentItem.right
			margins: 5
			leftMargin: spacing
		}

		TPButton {
			id: btn1
			text: "OK"
			autoSize: true
			enabled: txtPassword.getPassword().length > 4
			Layout.alignment: Qt.AlignHCenter
			onClicked: _passwdDlg.closePopup(0);
		}

		TPButton {
			id: btn2
			text: qsTr("Cancel")
			autoSize: true
			Layout.alignment: Qt.AlignHCenter
			onClicked: _passwdDlg.closePopup(1);
		}
	}
}
