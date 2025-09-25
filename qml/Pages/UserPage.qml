import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: userPage
	objectName: "userPage"
	implicitWidth: appSettings.pageWidth
	implicitHeight: appSettings.pageHeight

	property UserManager userManager
	property bool mainUserModified: false

	Connections {
		target: userModel
		function onUserModified(row: int, field: int) {
			if (row === 0)
				mainUserModified = true;
		}
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight
		anchors.fill: parent

		ColumnLayout {
			id: colMain
			anchors.fill: parent
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
				userRow: 0
				parentPage: userPage
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
		height: appSettings.itemDefaultHeight * 4

		TPButton {
			id: btnManageCoach
			text: qsTr("Manage coach(es)/trainer(s)")
			autoSize: true
			visible: userModel.mainUserIsClient

			anchors {
				horizontalCenter: parent.horizontalCenter
				top: parent.top
				topMargin: 5
			}

			onClicked: userManager.getCoachesPage();
		}

		TPButton {
			id: btnManageClients
			text: qsTr("Manage clients")
			autoSize: true
			visible: userModel.mainUserIsCoach

			anchors {
				horizontalCenter: parent.horizontalCenter
				bottom: parent.bottom
				bottomMargin: 15
			}

			onClicked: userManager.getClientsPage();
		}
	}

	function avatarChangedBySexSelection(row: int): void {
		usrProfile.defaultAvatarChanged(row);
	}

	function whenPageActivated(): void {
		//If there was no internet when a previous user page interaction happened, cancel all the pending requests because new ones might eventually be queued.
		userModel.cancelPendingOnlineRequests();
	}

	function whenPageDeActivated(): void {
		if (mainUserModified) {
			userModel.setMainUserConfigurationFinished();
			mainUserModified = false;
		}
	}
}
