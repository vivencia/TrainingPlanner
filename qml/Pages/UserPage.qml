import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.User

TPPage {
	id: userPage
	objectName: "userPage"
	imageSource: AppSettings.userBackground
	backgroundOpacity: 0.6
	implicitWidth: AppSettings.pageWidth
	implicitHeight: AppSettings.pageHeight

//public:
	property UserManager userManager

//private:
	property bool mainUserModified: false

	Connections {
		target: AppUserModel
		function onUserModified(row: int, field: int) {
			if (row === 0)
				userPage.mainUserModified = true;
		}
	}

	TPScrollView {
		parentPage: userPage
		contentHeight: colMain.implicitHeight
		anchors.fill: parent

		ColumnLayout {
			id: colMain
			anchors.fill: parent
			anchors.margins: 10
			spacing: 10

			TPLabel {
				text: qsTr("User Settings")
				font: AppGlobals.extraLargeFont
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
				Layout.leftMargin: 20
				Layout.rightMargin: 20
				Layout.topMargin: 20
			}

			UserPersonalData {
				id: usrData
				parentPage: userPage
				userRow: 0
				Layout.fillWidth: true
				Layout.margins: 10
			}

			UserContact {
				id: usrContact
				userRow: 0
				Layout.fillWidth: true
				Layout.margins: 10
			}

			UserCoach {
				id: usrCoach
				userRow: 0
				parentPage: userPage
				Layout.fillWidth: true
				Layout.margins: 10
			}

			UserProfile {
				id: usrProfile
				userRow: 0
				parentPage: userPage
				Layout.fillWidth: true
				Layout.margins: 10
			}
		}
	}

	footer: TPToolBar {
		height: AppSettings.itemDefaultHeight * 3

		ColumnLayout {
			spacing: 10
			anchors {
				fill: parent
				topMargin: 10
				bottomMargin: 20
			}

			TPButton {
				id: btnManageCoach
				text: qsTr("Manage coach(es)/trainer(s)")
				visible: AppUserModel.mainUserIsClient
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: parent.width - 20

				onClicked: userPage.userManager.getCoachesPage();
			}

			TPButton {
				id: btnManageClients
				text: qsTr("Manage clients")
				visible: AppUserModel.mainUserIsCoach
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: parent.width - 20

				onClicked: userPage.userManager.getClientsPage();
			}
		}
	}

	function avatarChangedBySexSelection(row: int): void {
		usrProfile.defaultAvatarChanged(row);
	}

	function whenPageDeActivated(): void {
		if (mainUserModified) {
			AppUserModel.setMainUserConfigurationFinished();
			mainUserModified = false;
		}
	}
}
