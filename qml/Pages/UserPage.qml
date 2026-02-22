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
	imageSource: appSettings.userBackground
	backgroundOpacity: 0.6
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

	TPScrollView {
		parentPage: userPage
		navButtonsVisible: true
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
		height: appSettings.itemDefaultHeight * 3

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
				visible: userModel.mainUserIsClient
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: parent.width - 20

				onClicked: userManager.getCoachesPage();
			}

			TPButton {
				id: btnManageClients
				text: qsTr("Manage clients")
				visible: userModel.mainUserIsCoach
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: parent.width - 20

				onClicked: userManager.getClientsPage();
			}
		}
	}

	function avatarChangedBySexSelection(row: int): void {
		usrProfile.defaultAvatarChanged(row);
	}

	function whenPageDeActivated(): void {
		if (mainUserModified) {
			userModel.setMainUserConfigurationFinished();
			mainUserModified = false;
		}
	}
}
