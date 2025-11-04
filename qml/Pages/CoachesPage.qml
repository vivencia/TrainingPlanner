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
	imageSource: appSettings.coachesBackground
	backgroundOpacity: 0.6

	required property UserManager userManager
	property int userRow: -1

	onPageActivated: {
		if (coachesList.enabled) {
			tabbar.setCurrentIndex(0);
			coachesList.selectItem(coachesList.currentRow !== -1 ? coachesList.currentRow : 0);
		}
		else if (pendingCoachesList.enabled) {
			tabbar.setCurrentIndex(1);
			pendingCoachesList.selectItem(pendingCoachesList.currentRow !== -1 ? pendingCoachesList.currentRow : 0);
		}
	}

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
		height: appSettings.itemDefaultHeight

		TPTabButton {
			text: qsTr("Coaches or Trainers")
			enabled: coachesList.enabled

			onClicked: userRow = userModel.currentCoaches.getUserIdx();
		}
		TPTabButton {
			text: qsTr("Pending answers")
			enabled: pendingCoachesList.enabled

			onClicked: userRow = userModel.pendingCoachesResponses.getUserIdx();
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
		height: coachesPage.height * 0.2

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

			TPCoachesAndClientsList {
				id: coachesList
				listClients: false
				listCoaches: true
				buttonString: qsTr("Résumé")
				height: parent.height - appSettings.itemDefaultHeight - 5

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				onItemSelected: (userRow) => coachesPage.userRow = userRow;
				onButtonClicked: userModel.viewResume(userRow);
			} //TPCoachesAndClientsList: coachesList

			RowLayout {
				uniformCellSizes: true
				height: appSettings.itemDefaultHeight

				anchors {
					top: coachesList.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Remove")
					enabled: userRow != 0 && coachesList.enabled  && coachesList.currentIndex !== -1
					rounded: false
					autoSize: true
					Layout.alignment: Qt.AlignCenter

					onClicked: showRemoveMessage(false,
								qsTr("Remove ") + userModel.userName(userRow) + "?",
								qsTr("The coach will be notified of your decision, but might still contact you unless you block them"));
				}
			}
		} //Item

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			TPCoachesAndClientsList {
				id: pendingCoachesList
				allowNotConfirmed: true
				listClients: false
				listCoaches: true
				buttonString: qsTr("Résumé")
				height: parent.height - appSettings.itemDefaultHeight - 5

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				//Temporary users(not confirmed) will always have the same index: userModel.count() - 1, so we need
				//to reset the userRow property in order for it to get a onChanged signal
				onItemSelected: (userRow) => {
					coachesPage.userRow = -1;
					coachesPage.userRow = userRow;
				}
				onButtonClicked: userModel.viewResume(userRow);
			} //TPCoachesAndClientsList: pendingCoachesList

			RowLayout {
				uniformCellSizes: true
				height: appSettings.itemDefaultHeight
				enabled: pendingCoachesList.enabled && pendingCoachesList.currentIndex !== -1

				anchors {
					bottom: parent.bottom
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Accept")
					autoSize: true
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: {
						userModel.acceptUser(userModel.pendingCoachesResponses, pendingCoachesList.currentIndex);
						if (!pendingCoachesList.enabled) {
							if (coachesList.enabled)
								tabbar.setCurrentIndex(0);
						}
					}
				}
				TPButton {
					text: qsTr("Decline")
					autoSize: true
					rounded: false
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
		multiline: true
		rounded: false
		enabled: userModel.canConnectToServer

		onClicked: displayOnlineCoachesDialog();

		anchors {
			top: listsLayout.bottom
			topMargin: 20
			left: parent.left
			leftMargin: 10
			right: parent.right
			rightMargin: 10
		}
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight
		enabled: userRow !== -1

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
			anchors.margins: 10
			spacing: 10

			UserPersonalData {
				id: usrData
				userRow: userRow
				parentPage: coachesPage
				enabled: userRow > 0
				width: appSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userRow: userRow
				enabled: userRow > 0
				width: appSettings.pageWidth - 20
			}

			UserProfile {
				id: usrProfile
				userRow: userRow
				parentPage: coachesPage
				enabled: userRow > 0
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
					msgRemoveUser = component.createObject(coachesPage, { parentPage: coachesPage, imageSource: "remove", keepAbove: true,
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
		if (!decline) {
			userModel.removeUser(userRow);
			if (!coachesList.enabled)
				if (pendingCoachesList.enabled) {
					tabbar.setCurrentIndex(1);
			}
		}
		else {
			userModel.rejectUser(userModel.pendingCoachesResponses, pendingCoachesList.currentIndex);
			if (!pendingCoachesList.enabled) {
				if (coachesList.enabled)
					tabbar.setCurrentIndex(0);
			}
		}
	}

	property UserCoachRequest requestDlg: null
	function displayOnlineCoachesDialog(): void {
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
