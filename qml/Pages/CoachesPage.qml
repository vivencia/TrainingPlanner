import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: coachesPage
	objectName: "CoachesPage"

	required property UserManager userManager
	property int curRow: -1

	TPLabel {
		id: lblMain
		text: qsTr("Coaches or Trainers");
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
			text: qsTr("Coaches or Trainers")
			enabled: userModel.haveCoaches

			onClicked: curRow = Qt.binding(function() { return userModel.currentRow; });
		}
		TabButton {
			text: qsTr("Pending answers")
			enabled: userModel.pendingCoachesResponses.count > 0
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
		height: 0.3*coachesPage.height

		Item {
			enabled: userModel.haveCoaches
			Layout.fillWidth: true
			Layout.fillHeight: true

			ListView {
				id: coachesList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: userModel.coachesNames
				height: 0.9*parent.height

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: true; visible: pendingCoachesList.contentHeight > pendingCoachesList.height
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
						text: userModel.coachesNames[index]
						font.pixelSize: appSettings.fontSize
						fontSizeMode: Text.Fit
						leftPadding: 5
						bottomPadding: 2
					}

					background: Rectangle {
						color: index === coachesList.currentIndex ? appSettings.entrySelectedColor :
								(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
					}

					onClicked: {
						curRow = userModel.findUserByName(userModel.coachesNames[index]);
						userModel.currentRow = curRow;
						coachesList.currentIndex = index;
					}
				} //ItemDelegate

				Component.onCompleted: {
					if (userModel.haveCoaches) {
						userModel.currentRow = userModel.findUserByName(userModel.coachesNames[0]);
						coachesList.currentIndex = 0;
					}
				}
			} //ListView: coachesList
		}

		Item {
			enabled: userModel.pendingCoachesResponses.count > 0
			Layout.fillWidth: true
			Layout.fillHeight: true

			ListView {
				id: pendingCoachesList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: userModel.pendingCoachesResponses
				height: 0.9*parent.height

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: true; visible: pendingCoachesList.contentHeight > pendingCoachesList.height
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

					contentItem: Item {
						Text {
							id: txtCoachName
							text: display
							font.pixelSize: 0
							fontSizeMode: Text.Fit
							leftPadding: 5
							bottomPadding: 2
							width: parent.width*0.7

							anchors {
								verticalCenter: parent.verticalCenter
								left: parent.left
							}
						}

						TPButton {
							text: qsTr("Profile")
							autoResize: true

							anchors {
								verticalCenter: parent.verticalCenter
								left: txtClientName.right
								right: parent.right
							}

							onClicked: userModel.downloadResume(pendingCoachesList.currentIndex);
						}
					}

					background: Rectangle {
						color: index === pendingCoachesList.currentIndex ? appSettings.entrySelectedColor :
								(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
					}

					onClicked: {
						if (userModel.getTemporaryUserInfo(userModel.pendingCoachesResponses, index) > 0)
							pendingCoachesList.currentIndex = index;
					}
				} //ItemDelegate
			} //ListView: pendingCoachesList

			RowLayout {
				uniformCellSizes: true
				height: 25

				TPButton {
					text: qsTr("Accept")
					autoResize: true

					onClicked: userModel.acceptUser(userModel.pendingCoachesResponses, pendingClientsList.currentIndex);
				}
				TPButton {
					text: qsTr("Decline")
					autoResize: true

					onClicked: userModel.rejectUser(userModel.pendingCoachesResponses, pendingClientsList.currentIndex);
				}
			}
		}//Item
	} //StackLayout

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight
		enabled: userModel.haveCoaches

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
				parentPage: coachesPage
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
				parentPage: coachesPage
				enabled: curRow > 0
				width: appSettings.pageWidth - 20
			}
		}
	}

	property TPBalloonTip msgRemoveUser: null
	function showRemoveMessage() {
		if (!appSettings.alwaysAskConfirmation) {
			userManager.removeUser(curRow, showCoaches);
			return;
		}

		if (msgRemoveUser === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveUser = component.createObject(coachesPage, { parentPage: coachesPage, imageSource: "remove",
						title: "Placeholder", message: qsTr("This action cannot be undone."), button1Text: qsTr("Yes"), button2Text: qsTr("No") });
					msgRemoveUser.button1Clicked.connect(function () { userManager.removeUser(curRow, showCoaches); });
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
