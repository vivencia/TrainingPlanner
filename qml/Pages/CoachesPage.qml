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

	TPLabel {
		id: lblMain
		text: qsTr("Coaches or Trainers");
		font: AppGlobals.extraLargeFont
		width: coachesPage.width
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			topMargin: 10
			left: parent.left
			right: parent.right
		}
	}

	TabBar {
		id: tabbar
		contentWidth: width
		height: AppSettings.itemDefaultHeight

		TPTabButton {
			text: qsTr("Coaches or Trainers")
			enabled: coachesList.enabled
			onClicked: coachesPage.userIdx = coachesList.selectedUserIdx;
		}

		TPTabButton {
			text: qsTr("Pending answers")
			enabled: pendingCoachesList.enabled
			onClicked: coachesPage.userIdx = pendingCoachesList.selectedUserIdx;
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
		height: coachesPage.height * 0.25

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
				listCoaches: true
				listConfirmed: true
				perItemButtonString: qsTr("Resumè");
				height: parent.height - AppSettings.itemDefaultHeight - 5

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				onItemSelected: (userIdx) => coachesPage.userIdx = userIdx;
				onItemButtonClicked: (userIdx) => coachesPage.viewResume(userIdx);
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
				listCoaches: true
				perItemButtonString: qsTr("Résumé")
				height: parent.height - btnAccept.height - 10

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				onItemSelected: (userIdx) => coachesPage.userIdx = userIdx;
				onItemButtonClicked: (userIdx) => coachesPage.viewResume(userIdx);
			} //TPCoachesAndClientsList: pendingCoachesList

			RowLayout {
				uniformCellSizes: true
				height: btnAccept.height
				enabled: pendingCoachesList.currentRow !== -1

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
			left: parent.left
			leftMargin: 10
			right: parent.right
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
			AppUserModel.rejectUser(pendingCoachesList.selectedUserIdx);
			if (!pendingCoachesList.enabled) {
				if (coachesList.enabled)
					tabbar.setCurrentIndex(0);
			}
		}
	}

	Loader {
		id: viewResumeLoader
		active: false
		asynchronous: true
		property TPFileViewer _file_viewer

		sourceComponent: TPFileViewer {
			missingFileInfo: qsTr(`The coach's resumè file could not be found.
				You can try to download it by pressing the second button from the left on the bottom of the screen`)
			canDownloadOrGenerate: true
			onWindowStateChanged: (window_state) => {
				if (window_state === TPFileViewer.WS_NORMAL)
					viewResumeLoader.active = false;
			}
			Component.onCompleted: viewResumeLoader._file_viewer = this;
		}
	}
	function viewResume(user_idx: int): void {
		viewResumeLoader._file_viewer.fileName = AppUserModel.resume(user_idx);
		viewResumeLoader._file_viewer.startFullScreen();
		viewResumeLoader.active = true;
	}
}
