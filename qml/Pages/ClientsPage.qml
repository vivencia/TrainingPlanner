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
			onClicked: clientsPage.userIdx = clientsList.selectedUser;
		}

		TPTabButton {
			text: qsTr("Pending requests")
			onClicked: clientsPage.userIdx = pendingClientsList.selectedUser;
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
			listConfirmed: true
			parentPage: clientsPage
			Layout.fillWidth: true
			Layout.preferredHeight: parent.height * 0.3

			onItemSelected: (user_idx) => clientsPage.userIdx = user_idx;
		} //TPCoachesAndClientsList: clientsList

		Item {
			Layout.fillWidth: true
			Layout.preferredHeight: parent.height * 0.3

			TPCoachesAndClientsList {
				id: pendingClientsList
				listClients: true
				parentPage: clientsPage
				height: parent.height - AppSettings.itemDefaultHeight - 5

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				onItemSelected: (user_idx) => clientsPage.userIdx = user_idx;
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

				TPButton2 {
					text: qsTr("Accept")
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: AppUserModel.acceptUser(pendingClientsList.workingModel.selectedUser);
				}
				TPButton2 {
					text: qsTr("Decline")
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: pendingClientsList.workingModel.removeCurrent();
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
}
