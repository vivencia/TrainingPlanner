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
	width: appSettings.pageWidth
	height: appSettings.pageHeight

	property UserManager userManager
	property int useMode

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
				width: appSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userRow: 0
				width: appSettings.pageWidth - 20				
			}

			UserCoach {
				id: usrCoach
				userRow: 0
				width: appSettings.pageWidth - 20
				Layout.topMargin: 20
			}

			UserProfile {
				id: usrProfile
				userRow: 0
				parentPage: userPage
				width: appSettings.pageWidth - 20
				Layout.topMargin: 20
			}
		}
	}

	footer: TPToolBar {
		id: coachsClientsToolBar
		height: appSettings.pageHeight*0.15

		TPButton {
			id: btnManageCoach
			text: qsTr("Manage coach(es)/trainer(s)")
			flat: false
			visible: useMode >= 3

			anchors {
				horizontalCenter: parent.horizontalCenter
				top: parent.top
			}

			onClicked: userManager.getClientsOrCoachesPage(false, true);
		}

		TPButton {
			id: btnManageClients
			text: qsTr("Manage clients")
			flat: false
			visible: useMode === 2 || useMode === 4

			anchors {
				horizontalCenter: parent.horizontalCenter
				top: btnManageCoach.bottom
			}

			onClicked: userManager.getClientsOrCoachesPage(true, false);
		}
	}

	function avatarChangedBySexSelection(row: int): void {
		usrProfile.defaultAvatarChanged(row);
	}
}
