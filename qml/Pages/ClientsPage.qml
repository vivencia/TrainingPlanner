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

	required property UserManager userManager
	property int curRow

	onPageActivated: {
		if (listsLayout.currentIndex === 0)
			clientsList.selectItem(clientsList.clientRow !== -1 ? clientsList.clientRow : 0);
		else
			pendingClientsList.selectItem(userModel.pendingClientsRequests.count > 0 ? userModel.pendingClientsRequests.currentRow : 0);
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
		height: 30

		TPTabButton {
			text: qsTr("Clients")
			enabled: userModel.haveClients
			checked: tabbar.currentIndex === 0

			onClicked: curRow = userModel.findUserByName(userModel.clientsNames(clientsList.currentIndex));
		}
		TPTabButton {
			text: qsTr("Pending requests")
			enabled: userModel.pendingClientsRequests ? userModel.pendingClientsRequests.count > 0 : false
			checked: tabbar.currentIndex === 1

			onClicked: {
				curRow = userModel.getTemporaryUserInfo(userModel.pendingClientsRequests, userModel.pendingClientsRequests.currentRow);
				if (curRow > 0)
					pendingClientsList.currentIndex = userModel.pendingClientsRequests.currentRow;
			}
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
		height: 0.2*clientsPage.height

		anchors {
			top: tabbar.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		TPClientsList {
			id: clientsList
			buttonString: qsTr("Remove")
			Layout.fillWidth: true
			Layout.fillHeight: true

			onClientSelected: (userRow) => curRow = userRow;
			onButtonClicked: showRemoveMessage(false,
						qsTr("Remove ") + userModel.userName(curRow) + "?",
						qsTr("The client will be notified of your decision, but might still contact you unless you block them"));
		} //TPClientsList

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			ListView {
				id: pendingClientsList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				reuseItems: true
				model: userModel.pendingClientsRequests
				height: parent.height * 0.8
				enabled: userModel.pendingClientsRequests ? userModel.pendingClientsRequests.count > 0 : false

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: true; visible: pendingClientsList.contentHeight > pendingClientsList.height
				}

				anchors {
					top: parent.top
					left: parent.left
					right: parent.right
				}

				delegate: ItemDelegate {
					spacing: 0
					padding: 5
					width: pendingClientsList.width
					height: userSettings.itemDefaultHeight

					contentItem: Text {
						text: name
						font.pixelSize: userSettings.fontSize
						fontSizeMode: Text.Fit
						leftPadding: 5
						bottomPadding: 2
					}

					background: Rectangle {
						color: index === pendingClientsList.currentIndex ? userSettings.entrySelectedColor :
							(index % 2 === 0 ? userSettings.listEntryColor1 : userSettings.listEntryColor2)
					}

					onClicked: selectItem(index);
				} //ItemDelegate

				function selectItem(index: int): void {
					if (index !== currentIndex) {
						const newrow = userModel.getTemporaryUserInfo(userModel.pendingClientsRequests, index);
						if (newrow > 0) {
							curRow = -1;
							curRow = newrow;
							currentIndex = index;
						}
					}
				}
			} //ListView: pendingClientsList

			RowLayout {
				uniformCellSizes: true
				height: userSettings.itemDefaultHeight
				visible: userModel.pendingClientsRequests ? userModel.pendingClientsRequests.count > 0 : false

				anchors {
					top: pendingClientsList.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Accept")
					autoSize: true
					Layout.alignment: Qt.AlignCenter

					onClicked: userModel.acceptUser(userModel.pendingClientsRequests, pendingClientsList.currentIndex);
				}
				TPButton {
					text: qsTr("Decline")
					autoSize: true
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
		enabled: userModel.haveClients

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
			spacing: 10

			UserPersonalData {
				id: usrData
				userRow: curRow
				parentPage: clientsPage
				enabled: curRow > 0
				width: userSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userRow: curRow
				enabled: curRow > 0
				width: userSettings.pageWidth - 20
			}

			UserProfile {
				id: usrProfile
				userRow: curRow
				parentPage: clientsPage
				enabled: curRow > 0
				width: userSettings.pageWidth - 20
			}
		}
	}

	property TPBalloonTip msgRemoveUser: null
	function showRemoveMessage(decline: bool, Title: string, Message: string): void {
		if (!userSettings.alwaysAskConfirmation) {
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
			userModel.removeUser(curRow);
		else
			userModel.rejectUser(userModel.pendingClientsRequests, pendingClientsList.currentIndex);
	}
}
