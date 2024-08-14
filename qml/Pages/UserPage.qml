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

	onPageActivated: userModel.setCurrentViewedUser(0);

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
			visible: userModel.appUseMode >= 3
			Layout.alignment: Qt.AlignCenter

			onClicked: appDB.openClientsOrCoachesPage(true);
		}

		TPButton {
			id: btnManageClients
			text: qsTr("Manage clients")
			flat: false
			visible: userModel.appUseMode === 2 || userModel.appUseMode === 4
			Layout.alignment: Qt.AlignCenter

			onClicked: appDB.openClientsOrCoachesPage(false);
		}
	}

	function apply() {
		appDB.saveUser();
	}
}
