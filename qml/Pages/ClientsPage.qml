pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.User

TPPage {
	id: clientsPage
	objectName: "ClientsPage"
	imageSource: AppSettings.clientsBackground
	backgroundOpacity: 0.6

//private:
	property int userIdx

	TPLabel {
		id: lblMain
		text: qsTr("Clients");
		font: AppGlobals.extraLargeFont
		width: clientsPage.width
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			topMargin: 20
			left: parent.left
			right: parent.right
		}
	}

	TabBar {
		id: tabbar
		contentWidth: width
		height: AppSettings.itemDefaultHeight

		TPTabButton {
			text: qsTr("Clients")
			onClicked: clientsList.selectItem(clientsList.currentRow >= 0 ? clientsList.currentRow : 0);
		}

		TPTabButton {
			text: qsTr("Pending requests")
			onClicked: pendingClientsList.selectItem(pendingClientsList.currentRow >= 0 ? pendingClientsList.currentRow : 0);
		}

		anchors {
			top: lblMain.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		Component.onCompleted: clientsList.selectItem(clientsList.currentRow);
	}

	StackLayout {
		id: listsLayout
		currentIndex: tabbar.currentIndex
		height: clientsPage.height * 0.2

		anchors {
			top: tabbar.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		TPCoachesAndClientsList {
			id: clientsList
			listClients: true
			listCoaches: false
			listConfirmed: true
			buttonString: qsTr("Remove")
			Layout.fillWidth: true
			Layout.preferredHeight: parent.height * 0.3

			onItemSelected: (userIdx) => clientsPage.userIdx = userIdx;
			onButtonClicked: clientsPage.showRemoveMessage(false);
		} //TPCoachesAndClientsList: clientsList

		Item {
			Layout.fillWidth: true
			Layout.preferredHeight: parent.height * 0.3

			TPCoachesAndClientsList {
				id: pendingClientsList
				listClients: true
				height: parent.height - AppSettings.itemDefaultHeight - 5

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				onItemSelected: (userIdx) => clientsPage.userIdx = userIdx;
			} //TPCoachesAndClientsList: pendingClientsList

			RowLayout {
				uniformCellSizes: true
				height: AppSettings.itemDefaultHeight
				enabled: pendingClientsList.currentRow !== -1

				anchors {
					top: pendingClientsList.bottom
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Accept")
					autoSize: true
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: AppUserModel.acceptUser(pendingClientsList.selectedUserIdx);
				}
				TPButton {
					text: qsTr("Decline")
					autoSize: true
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: clientsPage.showRemoveMessage(true);
				}
			}
		}//Item
	} //StackLayout

	TPScrollView {
		parentPage: clientsPage
		navButtonsVisible: enabled
		contentHeight: colMain.implicitHeight
		enabled: clientsPage.userIdx > 0

		anchors {
			top: listsLayout.bottom
			topMargin: 10
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		ColumnLayout {
			id: colMain
			anchors.fill: parent
			anchors.margins: 10
			spacing: 10

			UserPersonalData {
				id: usrData
				userIdx: clientsPage.userIdx
				parentPage: clientsPage
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userIdx: clientsPage.userIdx
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}

			UserProfile {
				id: usrProfile
				userIdx: clientsPage.userIdx
				parentPage: clientsPage
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}
		}
	}

	Loader {
		id: removeUserDlgLoader
		asynchronous: true
		active: false

		property bool decline
		property TPBalloonTip _remove_dlg

		sourceComponent: TPBalloonTip {
			parentPage: clientsPage
			imageSource: "remove"
			keepAbove: true
			message: removeUserDlgLoader.decline ?
				 qsTr("The client will receive your reply, but might choose to send another request unless you block them") :
				qsTr("The client will be notified of your decision, but might still contact you unless you block them")
			onButton1Clicked: clientsPage.removeOrDecline(removeUserDlgLoader.decline);
			onClosed: removeUserDlgLoader.active = false;
			Component.onCompleted: removeUserDlgLoader._remove_dlg = this;
		}

		onLoaded: {
			if (decline)
				_remove_dlg.title = qsTr("Decline ") + AppUserModel.userName(pendingClientsList.selectedUserIdx) + "?";
			else
				_remove_dlg.title = qsTr("Remove ") + AppUserModel.userName(clientsList.selectedUserIdx) + "?";
			_remove_dlg.tpOpen();
		}
	}
	function showRemoveMessage(decline: bool): void {
		removeUserDlgLoader.decline = decline;
		removeUserDlgLoader.active = true;
	}

	function removeOrDecline(decline: bool) {
		if (!decline)
			AppUserModel.removeUser(clientsPage.userIdx);
		else
			AppUserModel.rejectUser(pendingClientsList.selectedUserIdx);
	}
}
