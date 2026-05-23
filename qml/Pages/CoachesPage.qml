pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.User

TPPage {
	id: coachesPage
	objectName: "CoachesPage"
	imageSource: AppSettings.coachesBackground
	backgroundOpacity: 0.6

	property int userIdx

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
		width: coachesPage.width
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: coachesPage.top
			topMargin: 20
			left: coachesPage.left
			right: coachesPage.right
		}
	}

	TabBar {
		id: tabbar
		contentWidth: width
		height: AppSettings.itemDefaultHeight

		TPTabButton {
			text: qsTr("Coaches or Trainers")
			enabled: coachesList.enabled

			onClicked: coachesPage.userIdx = AppUserModel.currentCoaches.getUserIdx();
		}

		TPTabButton {
			text: qsTr("Pending answers")
			enabled: pendingCoachesList.enabled

			onClicked: coachesPage.userIdx = AppUserModel.pendingCoachesResponses.getUserIdx();
		}

		anchors {
			top: lblMain.bottom
			topMargin: 5
			left: coachesPage.left
			leftMargin: 5
			right: coachesPage.right
			rightMargin: 5
		}
	}

	StackLayout {
		id: listsLayout
		currentIndex: tabbar.currentIndex
		height: coachesPage.height * 0.25

		anchors {
			top: tabbar.bottom
			topMargin: 5
			left: coachesPage.left
			leftMargin: 5
			right: coachesPage.right
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
				height: parent.height - AppSettings.itemDefaultHeight - 5

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				onItemSelected: (userIdx) => coachesPage.userIdx = userIdx;
				onButtonClicked: AppUserModel.viewResume(coachesPage.userIdx);
			} //TPCoachesAndClientsList: coachesList

			RowLayout {
				uniformCellSizes: true
				height: AppSettings.itemDefaultHeight

				anchors {
					top: coachesList.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
				}

				TPButton {
					text: qsTr("Remove")
					enabled: coachesPage.userIdx != 0 && coachesList.enabled  && coachesList.currentRow !== -1
					rounded: false
					autoSize: true
					Layout.alignment: Qt.AlignCenter

					onClicked: coachesPage.showRemoveMessage(false);
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
				height: parent.height - btnAccept.height - 10

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				//Temporary users(not confirmed) will always have the same index: AppUserModel.count() - 1, so we need
				//to reset the userIdx property in order for it to get a onChanged signal
				onItemSelected: (userIdx) => {
					coachesPage.userIdx = -1;
					coachesPage.userIdx = userIdx;
				}
				onButtonClicked: AppUserModel.viewResume(coachesPage.userIdx);
			} //TPCoachesAndClientsList: pendingCoachesList

			RowLayout {
				uniformCellSizes: true
				height: btnAccept.height
				enabled: pendingCoachesList.enabled && pendingCoachesList.currentRow !== -1

				anchors {
					bottom: parent.bottom
					left: parent.left
					right: parent.right
				}

				TPButton {
					id: btnAccept
					text: qsTr("Accept")
					autoSize: true
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: {
						AppUserModel.acceptUser(pendingCoachesList.selectedUserIdx);
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

					onClicked: coachesPage.showRemoveMessage(true);
				}
			}
		}//Item
	} //StackLayout

	TPButton {
		id: btnFindCoachOnline
		text: qsTr("Look online for available coaches");
		multiline: true
		rounded: false
		enabled: AppUserModel.canConnectToServer

		onClicked: coachesPage.displayOnlineCoachesDialog();

		anchors {
			top: listsLayout.bottom
			topMargin: 20
			left: coachesPage.left
			leftMargin: 10
			right: coachesPage.right
			rightMargin: 10
		}
	}

	TPScrollView {
		parentPage: coachesPage
		navButtonsVisible: enabled
		contentHeight: colMain.implicitHeight
		enabled: coachesPage.userIdx > 0

		anchors {
			top: btnFindCoachOnline.bottom
			topMargin: 10
			left: coachesPage.left
			right: coachesPage.right
			bottom: coachesPage.bottom
		}

		ColumnLayout {
			id: colMain
			anchors.fill: parent
			anchors.margins: 10
			spacing: 10

			UserPersonalData {
				id: usrData
				userIdx: coachesPage.userIdx
				parentPage: coachesPage
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userIdx: coachesPage.userIdx
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}

			UserProfile {
				id: usrProfile
				userIdx: coachesPage.userIdx
				parentPage: coachesPage
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}
		}
	}

	Loader {
		id: onlineCoachesDialogLoader
		asynchronous: true
		active: false

		property UserCoachRequest _coaches_dialog
		sourceComponent: UserCoachRequest {
			parentPage: coachesPage
			onClosed: onlineCoachesDialogLoader.active = false;
			Component.onCompleted: onlineCoachesDialogLoader._coaches_dialog = this;
		}

		onLoaded: _coaches_dialog.tpOpen();
	}
	function displayOnlineCoachesDialog(): void {
		onlineCoachesDialogLoader.active = true;
	}

	Loader {
		id: removeUserDlgLoader
		asynchronous: true
		active: false

		property bool decline
		property TPBalloonTip _remove_dialog

		sourceComponent: TPBalloonTip {
			parentPage: coachesPage
			imageSource: "remove"
			keepAbove: true
			message: removeUserDlgLoader.decline ?
				qsTr("The coach will receive your reply, but might choose to send another answer unless you block them") :
				qsTr("The coach will be notified of your decision, but might still contact you unless you block them")
			onButton1Clicked: coachesPage.removeOrDecline(removeUserDlgLoader.decline);
			onClosed: removeUserDlgLoader.active = false;
			Component.onCompleted: removeUserDlgLoader._remove_dialog = this;
		}

		onLoaded: {
			if (decline)
				_remove_dialog.title = qsTr("Decline ") + AppUserModel.userName(pendingCoachesList.selectedUserIdx) + "?";
			else
				_remove_dialog.title = qsTr("Remove ") + AppUserModel.userName(coachesList.selectedUserIdx) + "?";
			_remove_dialog.tpOpen();
		}
	}
	function showRemoveMessage(decline: bool): void {
		removeUserDlgLoader.decline = decline;
		removeUserDlgLoader.active = true;
	}

	function removeOrDecline(decline: bool) {
		if (!decline) {
			AppUserModel.removeUser(coachesPage.userIdx);
			if (!coachesList.enabled)
				if (pendingCoachesList.enabled) {
					tabbar.setCurrentIndex(1);
			}
		}
		else {
			AppUserModel.rejectUser(ApendingCoachesList.selectedUserIdx);
			if (!pendingCoachesList.enabled) {
				if (coachesList.enabled)
					tabbar.setCurrentIndex(0);
			}
		}
	}
}
