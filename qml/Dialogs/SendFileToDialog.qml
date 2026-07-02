import QtQuick

import TpQml
import TpQml.Widgets
import TpQml.User

TPPopup {
	id: _dialog
	width: AppSettings.pageWidth - 20
	height: AppSettings.pageHeight * 0.4
	keepAbove: true
	useShape: true
	showBorder: true
	showTitleBar: true
	open_in_window: true
	canSlideToClose: true

//public:
	property int handle
	property string message
	property list<string> selectedUsers
	signal selectedOptions(int handle, list<string> selected_users, string message, bool present_dialog)

//private:
	readonly property int _button_size: width * 0.5 - 25

	onOpened: {
		usersList.reset();
		usersList.setSelectedUsers(selectedUsers);
	}
	onCloseActionExeced: selectedOptions(-1, ["no selection"], "", false);

	Column {
		id: mainLayout
		spacing: 5
		padding: 5
		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 15
			topMargin: 5
		}

		TPLabel {
			id: lblTitle
			text: qsTr("Send file to...")
			horizontalAlignment: Text.AlignHCenter
			height: AppSettings.itemDefaultHeight
			width: parent.width - AppSettings.itemDefaultHeight - 5
		}

		TPCoachesAndClientsList {
			id: usersList
			listClients: true
			listCoaches: true
			listConfirmed: true
			enabled: _dialog.selectedUsers.length === 0
			width: parent.width
			height: parent.height - txtMessage.height - 4 * AppSettings.itemDefaultHeight
		}

		TPLabel {
			text: qsTr("Message:")
			width: parent.width
			height: AppSettings.itemDefaultHeight
		}

		TPTextInput {
			id: txtMessage
			text: _dialog.message
			showClearTextButton: true
			width: parent.width
		}

		Item {
			width: parent.width
			height: AppSettings.itemDefaultHeight

			states: [
				State {
					when: _dialog.handle == -1
					AnchorChanges {
						target: btn1
						anchors.horizontalCenter: undefined
						anchors.left: parent.left
					}
					AnchorChanges {
						target: btn2
						anchors.horizontalCenter: undefined
						anchors.right: parent.right
					}
				},
				State {
					when: _dialog.handle == AppUtils.MH_TPMESSAGES_MANAGER
					AnchorChanges {
						target: btn1
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.left: undefined
					}
				},
				State {
					when: _dialog.handle == AppUtils.MH_TPCHAT
					AnchorChanges {
						target: btn2
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.right: undefined
					}

				}
			]

			TPButton {
				id: btn1
				text: qsTr("Send directly")
				enabled: usersList.anySelected
				visible: _dialog.handle == -1 || _dialog.handle == AppUtils.MH_TPMESSAGES_MANAGER
				width: _dialog._button_size
				height: AppSettings.itemDefaultHeight
				anchors {
					verticalCenter: parent.verticalCenter
					leftMargin: 5
				}

				onClicked: {
					_dialog.selectedOptions(usersList.selectedUsers(), AppUtils.SFM_TPMESSAGESMANAGER, txtMessage.text, false);
					_dialog.close();
				}
			}

			TPButton {
				id: btn2
				text: qsTr("Send via chat")
				enabled: usersList.anySelected
				visible: _dialog.handle == -1 || _dialog.handle == AppUtils.SFM_TPCHAT
				width: _dialog._button_size
				height: AppSettings.itemDefaultHeight
				anchors {
					verticalCenter: parent.verticalCenter
					rightMargin: 5
				}

				onClicked: {
					_dialog.selectedOptions(usersList.selectedUsers(), AppUtils.SFM_TPCHAT, txtMessage.text, false);
					_dialog.close();
				}
			}
		} //Item
	}//Layout
} //TPPopup
