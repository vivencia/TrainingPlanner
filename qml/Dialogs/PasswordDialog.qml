pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Dialogs
import TpQml.Widgets

TPPopup {
	id: _dialog
	keepAbove: true
	dim: true
	width: AppSettings.pageWidth * 0.8
	height: titleBar.height + topRow.height + newPasswordLoader.height + txtPassword.height + chkStorePassword.height
																											+ bottomRow.height
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

	onCloseActionExeced: (action_type) => {
		if (action_type === 0) {
			switch (mode) {
				case ItemManager.DM_GET_PASSWORD:
					passwordAcquired(true, _dialog.request_id, txtPassword.getPassword(), chkStorePassword.isChecked);
					break;
				case ItemManager.DM_NEW_PASSWORD:
					passwordCreated(true, _dialog.request_id, txtNewPassword.getPassword(), chkStorePassword.isChecked);
					break;
				case ItemManager.DM_CHANGE_PASSWORD:
					passwordChanged(true, _dialog.request_id, txtPassword.getPassword(), txtNewPassword.getPassword(),
																							chkStorePassword.isChecked);
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
		text: _dialog.title
		horizontalAlignment: Text.AlignHCenter
		anchors {
			top: _dialog.contentItem.top
			left: _dialog.contentItem.left
			right: _dialog.contentItem.right
			margins: 5
			rightMargin: _dialog.titleBar.titleBarButtons.width
		}
	}

	RowLayout {
		id: topRow

		anchors {
			top: lblTitle.bottom
			left: _dialog.contentItem.left
			right: _dialog.contentItem.right
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
			text: _dialog.message
			singleLine: false
			horizontalAlignment: Text.AlignJustify
			visible: _dialog.message.length > 0
			Layout.preferredWidth: _dialog.width - imgElement.width - 20
		}
	}

	Loader {
		id: newPasswordLoader
		asynchronous: true
		active: _dialog.mode !== ItemManager.DM_GET_PASSWORD
		height: 0

		anchors {
			top: topRow.bottom
			left: _dialog.contentItem.left
			right: _dialog.contentItem.right
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

				Component.onCompleted: {
					_dialog.txtNewPassword = this;
					newPasswordLoader.height = height;
				}
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

				Component.onCompleted: _dialog.txtConfirmNewPassword = this;
			}
		}
	}

	TPPassword {
		id: txtPassword
		label: _dialog.show_password_label ? (_dialog.mode !== ItemManager.DM_CHANGE_PASSWORD
													? AppUserModel.passwordLabel : qsTr("Current password: ")) : ""
		visible: _dialog.mode !== ItemManager.DM_NEW_PASSWORD
		enabled: _dialog.txtConfirmNewPassword ? _dialog.txtConfirmNewPassword.passwordOK : true
		height: visible ? defaultHeight : 0

		anchors {
			top: newPasswordLoader.active ? newPasswordLoader.bottom : topRow.bottom
			left: _dialog.contentItem.left
			right: _dialog.contentItem.right
			margins: 5
		}

		onPasswordEntered: _dialog.closePopup(0);
	}

	TPRadioButtonOrCheckBox {
		id: chkStorePassword
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		text: qsTr("Save password")
		isChecked: _dialog.store_password
		visible: _dialog.show_store_option
		height: visible ? preferredHeight : 0

		anchors {
			bottom: bottomRow.top
			left: _dialog.contentItem.left
			right: _dialog.contentItem.right
			margins: 5
		}

		onClicked: _dialog.store_password = !_dialog.store_password;
	}

	Row {
		id: bottomRow
		spacing: (_dialog.width - btn1.width - btn2.width) / 3

		anchors {
			bottom: _dialog.contentItem.bottom
			left: _dialog.contentItem.left
			right: _dialog.contentItem.right
			margins: 5
			leftMargin: spacing
		}

		TPButton2 {
			id: btn1
			text: "OK"
			enabled: txtPassword.getPassword().length > 4
			Layout.alignment: Qt.AlignHCenter
			onButtonClicked: _dialog.closePopup(TPPopup.DEFAULT_ACTION);
		}

		TPButton2 {
			id: btn2
			text: qsTr("Cancel")
			Layout.alignment: Qt.AlignHCenter
			onButtonClicked: _dialog.closePopup(TPPopup.BTN_CLOSE);
		}
	}
}
