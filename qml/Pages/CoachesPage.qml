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

//private:
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
			onClicked: coachesPage.userIdx = coachesList.workingModel.selectedUser;
		}

		TPTabButton {
			text: qsTr("Pending answers")
			onClicked: coachesPage.userIdx = pendingCoachesList.workingModel.selectedUser;
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

		TPCoachesAndClientsList {
			id: coachesList
			listCoaches: true
			listConfirmed: true
			parentPage: coachesPage
			perItemButtonString: qsTr("Resumè");
			Layout.fillWidth: true
			Layout.preferredHeight: parent.height * 0.3

			onItemSelected: (user_idx) => coachesPage.userIdx = user_idx;
			onItemButtonClicked: (user_idx) => coachesPage.viewResume(user_idx);
		} //TPCoachesAndClientsList: coachesList

		Item {
			Layout.fillWidth: true
			Layout.preferredHeight: parent.height * 0.3

			TPCoachesAndClientsList {
				id: pendingCoachesList
				listCoaches: true
				parentPage: coachesPage
				perItemButtonString: qsTr("Résumé")
				height: parent.height - AppSettings.itemDefaultHeight - 10

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
				height: AppSettings.itemDefaultHeight
				enabled: pendingCoachesList.workingModel.selectedUser !== -1

				anchors {
					top: pendingCoachesList.bottom
					left: parent.left
					right: parent.right
				}

				TPButton2 {
					text: qsTr("Accept")
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: AppUserModel.acceptUser(pendingCoachesList.workingModel.selectedUser);
				}
				TPButton2 {
					text: qsTr("Decline")
					rounded: false
					Layout.alignment: Qt.AlignCenter

					onClicked: pendingCoachesList.workingModel.removeCurrent();
				}
			}
		}//Item
	} //StackLayout

	TPButton2 {
		id: btnFindCoachOnline
		text: qsTr("Look online for available coaches");
		multiLine: true
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
		id: viewResumeLoader
		active: false
		asynchronous: true
		property TPFileViewer _file_viewer

		sourceComponent: TPFileViewer {
			fileOps: FileOperations {
				canDownloadOrGenerate: true
				fileName: AppUserModel.resume(coachesPage.userIdx);
			}

			missingFileInfo: qsTr(`The coach's resumè file could not be found.
				You can try to download it by pressing the second button from the left on the bottom of the screen`)

			onWindowStateChanged: (window_state) => {
				if (window_state === TPFileViewer.WS_NORMAL)
					viewResumeLoader.active = false;
			}
			Component.onCompleted: viewResumeLoader._file_viewer = this;
		}
	}

	function viewResume(user_idx: int): void {
		viewResumeLoader._file_viewer.startFullScreen();
		viewResumeLoader.active = true;
	}
}
