import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.User

TPPopup {
	id: _dialog
	width: AppSettings.pageWidth - 20
	height: AppSettings.pageHeight * 0.5
	useShape: true
	showBorder: true
	showTitleBar: true
	open_in_window: true
	canSlideToClose: true

//public:
	property int handle
	property string message
	signal selectedOptions(int handle, list<string> selected_users, string message)

	onOpened: usersList.reset();
	onCloseActionExeced: selectedOptions(-1, ["no selection"], "");

	ColumnLayout {
		spacing: 5
		anchors.fill: parent
		anchors.margins: 5

		TPLabel {
			id: lblTitle
			text: qsTr("Send file to")
			horizontalAlignment: Text.AlignHCenter
			Layout.preferredWidth: parent.width - _dialog.btnClose.width - 5
		}

		TPCoachesAndClientsList {
			id: usersList
			listClients: true
			listCoaches: true
			Layout.fillWidth: true
			Layout.preferredHeight: parent.height - txtMessage.height - (2 * buttonsRow.height) - 10
		}

		TPLabel {
			text: qsTr("Message:")
			Layout.fillWidth: true
		}

		TPTextInput {
			id: txtMessage
			text: _dialog.message
			showClearTextButton: true
			Layout.fillWidth: true
		}

		Row {
			id: buttonsRow
			spacing: 10
			Layout.fillWidth: true

			readonly property int buttonSize: parent.width * 0.5 - 5

			TPButton {
				text: qsTr("Send directly")
				enabled: usersList.anySelected
				visible: _dialog.handle == -1 || _dialog.handle == AppUtils.SFM_TPMESSAGESMANAGER
				width: buttonsRow.buttonSize
				height: AppSettings.itemDefaultHeight

				onClicked: {
					_dialog.selectedOptions(usersList.selectedUsers(), AppUtils.SFM_TPMESSAGESMANAGER, txtMessage.text);
					_dialog.close();
				}
			}

			TPButton {
				text: qsTr("Send via chat")
				enabled: usersList.anySelected
				visible: _dialog.handle == -1 || _dialog.handle == AppUtils.SFM_TPCHAT
				width: buttonsRow.buttonSize
				height: AppSettings.itemDefaultHeight

				onClicked: {
					_dialog.selectedOptions(usersList.selectedUsers(), AppUtils.SFM_TPCHAT, txtMessage.text);
					_dialog.close();
				}
			}
		} //Row
	}//Layout
} //TPPopup
