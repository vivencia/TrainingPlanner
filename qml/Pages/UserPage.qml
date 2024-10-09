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

	property QmlItemManager itemManager
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
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
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
				Layout.topMargin: -30
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

	footer: ToolBar {
		id: coachsClientsToolBar
		width: parent.width
		height: 70

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			id: btnManageCoach
			text: qsTr("Manage coach(es)/trainer(s)")
			flat: false
			visible: useMode >= 3

			anchors {
				horizontalCenter: parent.horizontalCenter
				top: parent.top
			}

			onClicked: itemManager.getClientsOrCoachesPage(false, true);
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

			onClicked: itemManager.getClientsOrCoachesPage(true, false);
		}
	}
}
