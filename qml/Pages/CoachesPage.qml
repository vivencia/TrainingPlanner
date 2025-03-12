import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"
import "../TPWidgets"
import "../User"

TPPage {
	id: coachesPage
	objectName: "CoachesPage"

	required property UserManager userManager
	property int curRow: userModel.currentRow

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

			onClicked: {
				curRow = userModel.getTemporaryUserInfo(userModel.pendingCoachesResponses, userModel.pendingCoachesResponses.currentRow);
				if (curRow > 0)
					pendingCoachesList.currentIndex = userModel.pendingCoachesResponses.currentRow;
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
		height: 0.2*coachesPage.height

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
				id: coachesList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: userModel.coachesNames
				height: 0.8*parent.height
				enabled: userModel.haveCoaches

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
							text: userModel.coachesNames[index]
							font.pixelSize: appSettings.fontSize
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
							text: qsTr("Résumé")
							autoResize: true

							anchors {
								verticalCenter: parent.verticalCenter
								left: txtCoachName.right
								right: parent.right
							}

							onClicked: userModel.viewResume(curRow);
						}
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

			RowLayout {
				uniformCellSizes: true
				height: 25
				visible: userModel.haveCoaches

				anchors {
					top: coachesList.bottom
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
								qsTr("The coach will be notified of your decision, but might still contact you unless you block them"));
				}
			}
		} //Item

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			ListView {
				id: pendingCoachesList
				contentHeight: availableHeight
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: userModel.pendingCoachesResponses
				height: 0.8*parent.height
				enabled: userModel.pendingCoachesResponses.count > 0

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
					width: pendingCoachesList.width
					height: 25

					contentItem: Item {
						Text {
							id: txtPendingCoachName
							text: name
							font.pixelSize: appSettings.fontSize
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
							text: qsTr("Résumé")
							autoResize: true

							anchors {
								verticalCenter: parent.verticalCenter
								left: txtPendingCoachName.right
								right: parent.right
							}

							onClicked: userModel.viewResume(curRow);
						}
					}

					background: Rectangle {
						color: index === pendingCoachesList.currentIndex ? appSettings.entrySelectedColor :
								(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
					}

					onClicked: {
						curRow = userModel.getTemporaryUserInfo(userModel.pendingCoachesResponses, index);
						if (curRow > 0)
							pendingCoachesList.currentIndex = index;
					}
				} //ItemDelegate
			} //ListView: pendingCoachesList

			RowLayout {
				uniformCellSizes: true
				height: 25
				visible: userModel.pendingCoachesResponses.count > 0

				anchors {
					top: pendingCoachesList.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Accept")
					autoResize: true
					Layout.alignment: Qt.AlignCenter

					onClicked: userModel.acceptUser(userModel.pendingCoachesResponses, pendingCoachesList.currentIndex);
				}
				TPButton {
					text: qsTr("Decline")
					autoResize: true
					Layout.alignment: Qt.AlignCenter

					onClicked: showRemoveMessage(true,
								qsTr("Decline ") + userModel.pendingCoachesResponses.display(userModel.pendingCoachesResponses.currentRow) + "?",
								qsTr("The coach will receive your reply, but might choose to send another answer unless you block them"));
				}
			}
		}//Item
	} //StackLayout

	TPButton {
		id: btnFindCoachOnline
		text: qsTr("Look online for available coaches");
		autoResize: true
		fixedSize: true
		height: 25

		onClicked: displayOnlineCoachesMenu();

		anchors {
			top: listsLayout.bottom
			topMargin: 10
			horizontalCenter: parent.horizontalCenter
		}
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight
		enabled: userModel.haveCoaches

		anchors {
			top: btnFindCoachOnline.bottom
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
	function showRemoveMessage(decline: bool, Title: string, Message: string): void {
		if (!appSettings.alwaysAskConfirmation) {
			removeOrDecline(decline);
			return;
		}

		if (msgRemoveUser === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveUser = component.createObject(coachesPage, { parentPage: coachesPage, imageSource: "remove", title: Title, message: Message });
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
			userManager.removeCoach(curRow);
		else
			userModel.rejectUser(userModel.pendingCoachesResponses, pendingCoachesList.currentIndex);
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

	property UserCoachRequest requestDlg: null
	function displayOnlineCoachesMenu(): void {
		if (requestDlg === null) {
			function createRequestDialog() {
				let component = Qt.createComponent("qrc:/qml/User/UserCoachRequest.qml", Qt.Asynchronous);

				function finishCreation() {
					requestDlg = component.createObject(coachesPage, { parentPage: coachesPage });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createRequestDialog();
		}
		requestDlg.show1(-1);
	}
}
