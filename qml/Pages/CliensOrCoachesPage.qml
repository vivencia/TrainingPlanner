import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import com.vivenciasoftware.qmlcomponents

TPPage {
	id: coachesOrClientsPage
	objectName: "coachesOrClientsPage"
	width: windowWidth
	height: windowHeight

	property bool showUsers
	property bool showCoaches
	property bool bModified: userModel.modified
	readonly property int moduleHeight: usrData.implicitHeight
	property TPBalloonTip msgRemoveUser: null

	onPageActivated: userModel.findFirstUser(bCoach);

	Row {
		id: controlsRow
		spacing: 10
		padding: 5
		height: 30

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		RoundButton {
			id: btnAdd
			icon.source: "qrc:/images/user-add.png"

			onClicked: userModel.addUser(showCoaches);
		}

		RoundButton {
			id: btnDel
			icon.source: "qrc:/images/user-del.png"

			onClicked: showRemoveMessage();
		}

		RoundButton {
			id: btnFirst
			icon.source: "qrc:/images/first.png"

			onClicked: userModel.setCurrentViewedUser(userModel.findFirstUser(showCoaches));
		}

		RoundButton {
			id: btnPrev
			icon.source: "qrc:/images/prev.png"

			onClicked: userModel.setCurrentViewedUser(userModel.findPrevUser(showCoaches));
		}

		RoundButton {
			id: btnNext
			icon.source: "qrc:/images/next.png"

			onClicked: userModel.setCurrentViewedUser(userModel.findNextUser(showCoaches));
		}

		RoundButton {
			id: btnLast
			icon.source: "qrc:/images/last.png"

			onClicked: userModel.setCurrentViewedUser(userModel.findLastUser(showCoaches));
		}
	}

	TPCheckBox {
		id: chkCurrent
		text: showCoaches ? qsTr("Default Coach/Trainer") : qsTr("Default Client")
		height: 25

		anchors {
			top: controlsRow.bottom
			left: parent.left
			right: parent.right
		}

		onCheckedChanged: {
			if (showCoaches)
				userModel.currentCoach = checked ? userModel.currentViewedUser() : 0;
			else
				userModel.currentUser = checked ? userModel.currentViewedUser() : 0;
		}

		onClicked: bCoachOK = userModel.coach !== 2;

		Component.onCompleted: {
			if (showCoaches)
				checked = userModel.currentCoach === userModel.currentViewedUser();
			else
				checked = userModel.currentUser === userModel.currentViewedUser();
		}
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight

		anchors {
			top: chkCurrent.bottom
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		ColumnLayout {
			id: colMain
			anchors.fill: parent
			spacing: 10

			Label {
				text: showCoaches ? qsTr("Coaches or Trainers") : qsTr("Clients")
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

	property TPBalloonTip msgRemoveUser: null
	function showRemoveMessage() {
		if (!AppSettings.alwaysAskConfirmation) {
			userModel.removeUser(userModel.currentViewedUser());
			return;
		}

		if (msgRemoveUser === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveUser = component.createObject(trainingDayPage, { parentPage: coachesOrClientsPage, imageSource: "remove.png",
						message: qsTr("This action cannot be undone."), button1Text: qsTr("Yes"), button2Text: qsTr("No") } );
					msgRemoveUser.button1Clicked.connect(function () { userModel.removeUser(userModel.currentViewedUser()); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		msgRemoveUser.title = qsTr("Remove ") + userModel.userName + "?"
		msgRemoveUser.show(-1);
	}
}
