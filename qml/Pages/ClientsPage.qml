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

			onClicked: curRow = Qt.binding(function() { return userModel.getTemporaryUserInfo(
										userModel.pendingClientsRequests, userModel.pendingClientsRequests.currentRow); });
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
		height: 0.3*clientsPage.height

		Item {
			enabled: userModel.haveClients
			Layout.fillWidth: true
			Layout.fillHeight: true

			ListView {
				id: clientsList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: userModel.clientsNames
				height: 0.9*parent.height

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
					width: parent.width
					height: 25

					contentItem: Text {
						text: display
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
		}

		Item {
			enabled: userModel.pendingClientsRequests.count > 0
			Layout.fillWidth: true
			Layout.fillHeight: true

			ListView {
				id: pendingClientsList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: userModel.pendingClientsRequests
				height: 0.9*parent.height

				onModelChanged: console.log(userModel.pendingClientsRequests.count);
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
					width: parent.width
					height: 25

					contentItem: Text {
						text: model.name
						font.pixelSize: 0
						fontSizeMode: Text.Fit
						leftPadding: 5
						bottomPadding: 2
					}

					background: Rectangle {
						color: index === pendingClientsList.currentIndex ? appSettings.entrySelectedColor :
							(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
					}

					onClicked: {
						if (userModel.getTemporaryUserInfo(userModel.pendingClientsRequests, index) > 0)
							pendingClientsList.currentIndex = index;
					}
				} //ItemDelegate
			} //ListView: pendingClientsList

			RowLayout {
				uniformCellSizes: true
				height: 25

				anchors {
					top: pendingClientsList.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Accept")
					autoResize: true

					onClicked: userModel.acceptUser(userModel.pendingClientsRequests, pendingClientsList.currentIndex);
				}
				TPButton {
					text: qsTr("Decline")
					autoResize: true

					onClicked: userModel.rejectUser(userModel.pendingClientsRequests, pendingClientsList.currentIndex);
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
	function showRemoveMessage() {
		if (!appSettings.alwaysAskConfirmation) {
			userManager.removeUser(curRow, showClients);
			return;
		}

		if (msgRemoveUser === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveUser = component.createObject(clientsPage, { parentPage: clientsPage, imageSource: "remove",
						title: "Placeholder", message: qsTr("This action cannot be undone."), button1Text: qsTr("Yes"), button2Text: qsTr("No") });
					msgRemoveUser.button1Clicked.connect(function () { userManager.removeUser(curRow, showClients); });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		msgRemoveUser.title = qsTr("Remove ") + userModel.userName(curRow) + "?"
		msgRemoveUser.show(-1);
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
