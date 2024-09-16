import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import com.vivenciasoftware.qmlcomponents

TPPage {
	id: userPage
	objectName: "userPage"
	width: windowWidth
	height: windowHeight

	property int useMode
	property bool bModified: userModel.modified
	readonly property int moduleHeight: usrContact.moduleHeight

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

			Label {
				text: qsTr("User Settings")
				color: AppSettings.fontColor
				font.bold: true
				font.pointSize: AppSettings.fontSizeTitle
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
				Layout.topMargin: 20
			}

			UserPersonalData {
				id: usrData
				userRow: 0
				parentPage: userPage
				width: windowWidth - 20
				height: moduleHeight
			}

			UserContact {
				id: usrContact
				userRow: 0
				width: windowWidth - 20
				Layout.topMargin: -30
			}

			UserCoach {
				id: usrCoach
				userRow: 0
				width: windowWidth - 20
				height: moduleHeight
				Layout.topMargin: 20
			}

			UserProfile {
				id: usrProfile
				userRow: 0
				parentPage: userPage
				width: windowWidth - 20
				height: moduleHeight
			}
		}
	}

	footer: ToolBar {
		id: coachsClientsToolBar
		width: parent.width
		height: footerHeight

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
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

			onClicked: appControl.getClientsOrCoachesPage(false, true);
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

			onClicked: appControl.getClientsOrCoachesPage(true, false);
		}
	}

	function apply() {
		appDB.saveUser(0);
	}
}
