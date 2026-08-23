import QtQuick

import TpQml
import TpQml.User
import TpQml.Widgets

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
	property list<string> selectedUsers
	signal sendMessage(selected_users: list<string>, message: string, include_file: string)

//private:
	readonly property string _no_file_chosen: qsTr("No file selected")
	property string _selected_file

	onOpened: {
		usersList.reset();
		usersList.setSelectedUsers(selectedUsers);
		_selected_file = _no_file_chosen;
	}

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
			text: qsTr("Send message to...")
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
			showClearTextButton: true
			width: parent.width
		}

		Row {
			width: parent.width

			TPFileDialog {
				id: chooseFileDlg
				title: qsTr("Choose a file to include in the new message")
				onDialogClosed: (result) => {
					if (result)
						_dialog._selected_file = AppUtils.getCorrectPath(currentFile);
				}
			}

			TPLabel {
				text: _dialog._selected_file
				elide: Text.ElideMiddle
				width: parent.width - btnChoose.width - 20
			}
			TPButton {
				id: btnChoose
				imageSource: "choose-flle"
				onClicked: chooseFileDlg.open();
			}
		}

		TPButton {
			id: btnSend
			text: qsTr("Send")
			enabled: usersList.anySelected && txtMessage.text.length > 0
			autoSize: true

			onClicked: {
				_dialog.sendMessage(usersList.selectedUsers(), txtMessage.text, _dialog._selected_file);
				_dialog.close();
			}
		}

		TPButton {
			id: btnClose
			text: qsTr("Cancel")
			autoSize: true
			onClicked: _dialog.close();
		}
	}//Layout
} //TPPopup
