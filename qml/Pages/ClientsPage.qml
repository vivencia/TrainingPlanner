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
	property int curRow: userModel.currentRow

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

		TabButton {
			text: qsTr("Clients")
			enabled: userModel.haveClients

			onClicked: curRow = Qt.binding(function() { return userModel.currentRow; });
		}
		TabButton {
			text: qsTr("Pending requests")
			enabled: userModel.pendingClientsRequests.count > 0

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

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			ListView {
				id: clientsList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: userModel.clientsNames
				height: 0.8*parent.height
				enabled: userModel.haveClients

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: true; visible: clientsList.contentHeight > clientsList.height
				}

				anchors {
					top: parent.top
					left: parent.left
					right: parent.right
				}

				delegate: ItemDelegate {
					spacing: 0
					padding: 5
					width: parent.width
					height: 25

					contentItem: Text {
						text: modelData
						font.pixelSize: appSettings.fontSize
						fontSizeMode: Text.Fit
						leftPadding: 5
						bottomPadding: 2
					}

					background: Rectangle {
						color: index === clientsList.currentIndex ? appSettings.entrySelectedColor :
								(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
					}

					onClicked: {
						curRow = userModel.findUserByName(userModel.clientsNames[index]);
						userModel.currentRow = curRow;
						clientsList.currentIndex = index;
					}
				} //ItemDelegate

				Component.onCompleted: {
					if (userModel.haveClients) {
						userModel.currentRow = userModel.findUserByName(userModel.clientsNames[0]);
						clientsList.currentIndex = 0;
					}
				}
			} //ListView: clientsList

			RowLayout {
				uniformCellSizes: true
				height: 25
				visible: userModel.haveClients

				anchors {
					top: clientsList.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Remove")
					autoResize: true
					enabled: curRow != 0
					Layout.alignment: Qt.AlignCenter

					onClicked: showRemoveMessage(false,
								qsTr("Remove ") + userModel.userName(curRow) + "?",
								qsTr("The client will be notified of your decision, but might still contact you unless you block them"));
				}
			}
		} //Item

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			ListView {
				id: pendingClientsList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: userModel.pendingClientsRequests
				height: 0.8*parent.height
				enabled: userModel.pendingClientsRequests.count > 0

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
					height: 25

					contentItem: Text {
						text: name
						font.pixelSize: appSettings.fontSize
						fontSizeMode: Text.Fit
						leftPadding: 5
						bottomPadding: 2
					}

					background: Rectangle {
						color: index === pendingClientsList.currentIndex ? appSettings.entrySelectedColor :
							(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
					}

					onClicked: {
						const newrow = userModel.getTemporaryUserInfo(userModel.pendingClientsRequests, index);
						if (newrow > 0) {
							curRow = -1;
							curRow = newrow;
							pendingClientsList.currentIndex = index;
						}
					}
				} //ItemDelegate
			} //ListView: pendingClientsList

			RowLayout {
				uniformCellSizes: true
				height: 25
				visible: userModel.pendingClientsRequests.count > 0

				anchors {
					top: pendingClientsList.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Accept")
					autoResize: true
					Layout.alignment: Qt.AlignCenter

					onClicked: userModel.acceptUser(userModel.pendingClientsRequests, pendingClientsList.currentIndex);
				}
				TPButton {
					text: qsTr("Decline")
					autoResize: true
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
				width: appSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userRow: curRow
				enabled: curRow > 0
				width: appSettings.pageWidth - 20
			}

			UserProfile {
				id: usrProfile
				userRow: curRow
				parentPage: clientsPage
				enabled: curRow > 0
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
					msgRemoveUser = component.createObject(clientsPage, { parentPage: clientsPage, imageSource: "remove", title: Title, message: Message });
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
			userManager.removeClient(curRow);
		else
			userModel.rejectUser(userModel.pendingClientsRequests, pendingClientsList.currentIndex);
	}

	function avatarChangedBySexSelection(row: int) {
		usrProfile.defaultAvatarChanged(row);
	}

	function whenPageActivated(): void {
		//Online data might change. Keep up with it
		usrData.getUserInfo();
		usrContact.getUserInfo();
		usrCoach.getUserInfo();
		usrProfile.getUserInfo();
	}
}
