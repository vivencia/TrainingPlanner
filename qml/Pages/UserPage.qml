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

	property bool bModified: userModel.modified
	readonly property int moduleHeight: usrProfile.implicitHeight

	onPageActivated: userModel.setCurrentViewedUser(0);

	ScrollView {
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight

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
				parentPage: userPage
				width: windowWidth - 20
				height: moduleHeight
			}

			UserContact {
				id: usrContact
				width: windowWidth - 20
				height: moduleHeight
			}

			UserCoach {
				id: usrCoach
				width: windowWidth - 20
				height: moduleHeight
			}

			UserProfile {
				id: usrProfile
				parentPage: userPage
				width: windowWidth - 20
			}

			TPButton {
				id: btnManageCoach
				text: qsTr("Manage coach(es)/trainer(s)")
				visible: userModel.coach >= 3
				Layout.alignment: Qt.AlignCenter

				onClicked: appDB.createClientsOrCoachesPage(true);
			}

			TPButton {
				id: btnManageClients
				text: qsTr("Manage clients")
				visible: userModel.coach === 2 || userModel.coach === 4
				Layout.alignment: Qt.AlignCenter

				onClicked: appDB.createClientsOrCoachesPage(false);
			}
		}
	}

	function apply() {
		appDB.saveUser();
	}
}
