import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: clientsPage
	objectName: "ClientsPage"
	imageSource: appSettings.clientsBackground
	backgroundOpacity: 0.8

	required property UserManager userManager
	property int userRow

	onPageActivated: {
		if (listsLayout.currentIndex === 0)
			clientsList.selectItem(clientsList.currentRow !== -1 ? clientsList.currentRow : 0);
		else
			pendingClientsList.selectItem(pendingClientsList.currentRow !== -1 ? pendingClientsList.currentRow : 0);
	}

	TPLabel {
		id: lblMain
		text: qsTr("Clients");
		font: AppGlobals.extraLargeFont
		width: parent.width
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
		height: appSettings.itemDefaultHeight

		TPTabButton {
			text: qsTr("Clients")
			enabled: clientsList.enabled

			onClicked: userRow = userModel.currentClients.getUserIdx();
		}

		TPTabButton {
			text: qsTr("Pending requests")
			enabled: pendingClientsList.enabled

			onClicked: userRow = userModel.pendingClientsRequests.getUserIdx();
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
			onButtonClicked: showRemoveMessage(false,
						qsTr("Remove ") + userModel.userName(clientsPage.userRow) + "?",
						qsTr("The client will be notified of your decision, but might still contact you unless you block them"));
		} //TPCoachesAndClientsList: clientsList

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			TPCoachesAndClientsList {
				id: pendingClientsList
				allowNotConfirmed: true
				listClients: true
				listCoaches: false

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				onItemSelected: (userRow) => clientsPage.userRow = userRow;
			} //TPCoachesAndClientsList: pendingClientsList

			RowLayout {
				uniformCellSizes: true
				height: appSettings.itemDefaultHeight
				enabled: userRow != 0 && pendingClientsList.enabled  && pendingClientsList.currentIndex !== -1

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

					onClicked: userModel.acceptUser(userModel.pendingClientsRequests, pendingClientsList.currentIndex);
				}
				TPButton {
					text: qsTr("Decline")
					autoSize: true
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: showRemoveMessage(true,
								qsTr("Decline ") + userModel.pendingClientsRequests.display(userModel.pendingClientsRequests.currentRow) + "?",
								qsTr("The client will receive your reply, but might choose to send another request unless you block them"));
				}
			}
		}//Item
	} //StackLayout

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight
		enabled: tabbar.currentItem.enabled

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
				enabled: clientsPage.userRow > 0
				width: appSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userRow: clientsPage.userRow
				enabled: clientsPage.userRow > 0
				width: appSettings.pageWidth - 20
			}

			UserProfile {
				id: usrProfile
				userRow: clientsPage.userRow
				parentPage: clientsPage
				enabled: clientsPage.userRow > 0
				width: appSettings.pageWidth - 20
			}
		}
	}

	property TPBalloonTip msgRemoveUser: null
	function showRemoveMessage(decline: bool, Title: string, Message: string): void {
		if (!appSettings.alwaysAskConfirmation) {
			removeOrDecline(decline);
			return;
		}

		if (msgRemoveUser === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveUser = component.createObject(clientsPage, { parentPage: clientsPage, imageSource: "remove", keepAbove: true,
															title: Title, message: Message });
					msgRemoveUser.button1Clicked.connect(function () { removeOrDecline(decline); });
					msgRemoveUser.show(-1);
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		else {
			msgRemoveUser.title = Title;
			msgRemoveUser.message = Message;
			msgRemoveUser.show(-1);
		}
	}

	function removeOrDecline(decline: bool) {
		if (!decline)
			userModel.removeUser(userRow);
		else
			userModel.rejectUser(userModel.pendingClientsRequests, pendingClientsList.currentIndex);
	}
}
