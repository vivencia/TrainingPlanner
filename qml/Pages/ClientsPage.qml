pragma componentBahavior: Bound

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

	property int userRow

	onPageActivated: {
		if (clientsList.enabled) {
			tabbar.setCurrentIndex(0);
			clientsList.selectItem(clientsList.currentRow !== -1 ? clientsList.currentRow : 0);
		}
		else if (pendingClientsList.enabled) {
			tabbar.setCurrentIndex(1);
			pendingClientsList.selectItem(pendingClientsList.currentRow !== -1 ? pendingClientsList.currentRow : 0);
		}
	}

	TPLabel {
		id: lblMain
		text: qsTr("Clients");
		font: AppGlobals.extraLargeFont
		width: clientsPage.width
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: clientsPage.top
			topMargin: 20
			left: clientsPage.left
			right: clientsPage.right
		}
	}

	TabBar {
		id: tabbar
		contentWidth: width
		height: AppSettings.itemDefaultHeight

		TPTabButton {
			text: qsTr("Clients")
			enabled: clientsList.enabled

			onClicked: clientsPage.userRow = AppUserModel.currentClients.getUserIdx();
		}

		TPTabButton {
			text: qsTr("Pending requests")
			enabled: pendingClientsList.enabled

			onClicked: clientsPage.userRow = AppUserModel.pendingClientsRequests.getUserIdx();
		}

		anchors {
			top: lblMain.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
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
			buttonString: qsTr("Remove")
			Layout.fillWidth: true
			Layout.fillHeight: true

			onItemSelected: (userRow) => clientsPage.userRow = userRow;
			onButtonClicked: showRemoveMessage(false);
		} //TPCoachesAndClientsList: clientsList

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			TPCoachesAndClientsList {
				id: pendingClientsList
				allowNotConfirmed: true
				listClients: true
				listCoaches: false
				height: parent.height - AppSettings.itemDefaultHeight - 5

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				//Temporary users(not confirmed) will always have the same index: AppUserModel.count() - 1, so we need
				//to reset the clientsPage.userRow property in order for it to get a onChanged signal
				onItemSelected: (userRow) => {
					clientsPage.userRow = -1;
					clientsPage.userRow = userRow;
				}
			} //TPCoachesAndClientsList: pendingClientsList

			RowLayout {
				uniformCellSizes: true
				height: AppSettings.itemDefaultHeight
				enabled: clientsPage.userRow != 0 && pendingClientsList.enabled  && pendingClientsList.currentIndex !== -1

				anchors {
					top: pendingClientsList.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Accept")
					autoSize: true
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: {
						AppUserModel.acceptUser(AppUserModel.pendingClientsRequests, pendingClientsList.currentIndex);
						if (!pendingClientsList.enabled) {
							if (clientsList.enabled)
								tabbar.setCurrentIndex(0);
						}
					}
				}
				TPButton {
					text: qsTr("Decline")
					autoSize: true
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: showRemoveMessage(true);
				}
			}
		}//Item
	} //StackLayout

	TPScrollView {
		parentPage: clientsPage
		navButtonsVisible: enabled
		contentHeight: colMain.implicitHeight
		enabled: clientsPage.userRow > 0

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
				userRow: clientsPage.userRow
				parentPage: clientsPage
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userRow: clientsPage.userRow
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}

			UserProfile {
				id: usrProfile
				userRow: clientsPage.userRow
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

		sourceComponent: TPBalloonTip {
			parentPage: clientsPage
			imageSource: "remove"
			keepAbove: true
			message: removeUserDlgLoader.decline ?
						 qsTr("The client will receive your reply, but might choose to send another request unless you block them") :
						 qsTr("The client will be notified of your decision, but might still contact you unless you block them")
			onButton1Clicked: clientsPage.removeOrDecline(removeUserDlgLoader.decline);
			onClosed: removeuserDlgLoader.active = false;
		}

		onLoaded: {
			if (decline)
				item.title = qsTr("Remove ") + AppUserModel.userName(clientsPage.userRow) + "?";
			else
				item.title = qsTr("Decline ") +
								AppUserModel.pendingClientsRequests.display(AppUserModel.pendingClientsRequests.currentRow) + "?";
			item.show(-1);
		}
	}
	function showRemoveMessage(decline: bool): void {
		removeUserDlgLoader.decline = decline;
		removeUserDlgLoader.active = true;
	}

	function removeOrDecline(decline: bool) {
		if (!decline) {
			AppUserModel.removeUser(clientsPage.userRow);
			if (!clientsList.enabled)
				if (pendingClientsList.enabled) {
					tabbar.setCurrentIndex(1);
			}
		}
		else {
			AppUserModel.rejectUser(AppUserModel.pendingClientsRequests, pendingClientsList.currentIndex);
			if (!pendingClientsList.enabled) {
				if (clientsList.enabled)
					tabbar.setCurrentIndex(0);
			}
		}
	}
}
