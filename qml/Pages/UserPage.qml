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
	readonly property int moduleHeight: usrProfile.moduleHeight

	Component.onCompleted: {
		userModeChanged(0);
		userModel.appUseModeChanged.connect(userModeChanged);
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			bottom: rowBtnsManage.top
		}

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
				height: moduleHeight
			}

			UserCoach {
				id: usrCoach
				userRow: 0
				width: windowWidth - 20
				height: moduleHeight
			}

			UserProfile {
				id: usrProfile
				userRow: 0
				parentPage: userPage
				width: windowWidth - 20
			}
		}
	}

	RowLayout {
		id: rowBtnsManage
		height: 50

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		TPButton {
			id: btnManageCoach
			text: qsTr("Manage coach(es)/trainer(s)")
			flat: false
			Layout.alignment: Qt.AlignCenter

			onClicked: appDB.openClientsOrCoachesPage(true);
		}

		TPButton {
			id: btnManageClients
			text: qsTr("Manage clients")
			flat: false
			Layout.alignment: Qt.AlignCenter

			onClicked: appDB.openClientsOrCoachesPage(false);
		}
	}

	function userModeChanged(row: int) {
		if (row !== 0)
			return;
		btnManageCoach.visible = userModel.appUseMode(0) >= 3;
		btnManageClients.visible = userModel.appUseMode(0) === 2 || userModel.appUseMode(0) === 4;
	}

	function apply() {
		appDB.saveUser(0);
	}
}
