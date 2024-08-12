import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import com.vivenciasoftware.qmlcomponents

TPPage {
	id: coachesPage
	objectName: "coachesPage"
	width: windowWidth
	height: windowHeight

	property bool bModified: userModel.modified
	readonly property int moduleHeight: usrData.implicitHeight

	onPageActivated: userModel.findFirstUser(true);

	Row {
		spacing: 10
		padding: 5

		RoundButton {
			id: btnAdd
			icon.source: "qrc:/images/user-add.png"
		}

		RoundButton {
			id: btnDel
			icon.source: "qrc:/images/user-del.png"
		}

		RoundButton {
			id: btnFirst
			icon.source: "qrc:/images/first.png"
		}

		RoundButton {
			id: btnPrev
			icon.source: "qrc:/images/prev.png"
		}

		RoundButton {
			id: btnNext
			icon.source: "qrc:/images/next.png"
		}

		RoundButton {
			id: btnLast
			icon.source: "qrc:/images/last.png"
		}
	}

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
				text: qsTr("Coaches or Trainers")
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
		}
	}

	function apply() {
		appDB.saveUser();
	}
}
