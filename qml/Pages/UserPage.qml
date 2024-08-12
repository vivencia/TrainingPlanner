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
	readonly property int moduleHeight: usrData.implicitHeight

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
				width: windowWidth - 20
				parentPage: userPage
			}

			UserContact {
				id: usrContact
				width: windowWidth - 20
				height: moduleHeight
			}

			UserProfile {
				id: usrProfile
				width: windowWidth - 20
				height: moduleHeight
				parentPage: userPage
			}

			UserCoach {
				id: usrCoach
				width: windowWidth - 20
				height: moduleHeight
			}

			TPButton {
				id: btnManageCoach
				text: qsTr("Manage coach(es)/trainer(s)")
				visible: userModel.coach >= 3
				Layout.alignment: Qt.AlignCenter

				onClicked: appDB.openCoachesPage();
			}

			TPButton {
				id: btnManageClients
				text: qsTr("Manage clients")
				visible: userModel.coach === 2
				Layout.alignment: Qt.AlignCenter

				onClicked: appDB.openClientsPage();
			}
		}
	}

	function apply() {
		appDB.saveUser();
	}
}
