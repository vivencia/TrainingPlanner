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

	property UserManager userManager
	property int useMode
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
			}

			UserContact {
				id: usrContact
				userRow: 0
				Layout.fillWidth: true
			}

			UserCoach {
				id: usrCoach
				userRow: 0
				parentPage: userPage
				Layout.fillWidth: true
			}

			UserProfile {
				id: usrProfile
				userRow: 0
				parentPage: userPage
				Layout.fillWidth: true
			}
		}
	}

	footer: TPToolBar {
		height: 1.2*footerHeight

		TPButton {
			id: btnManageCoach
			text: qsTr("Manage coach(es)/trainer(s)")
			flat: false
			autoResize: true
			visible: useMode >= 3

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
			flat: false
			autoResize: true
			visible: useMode === 2 || useMode === 4

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
