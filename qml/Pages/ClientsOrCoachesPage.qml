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

	property int curUserRow
	property bool showUsers
	property bool showCoaches
	property bool bModified: userModel.modified
	readonly property int moduleHeight: usrContact.moduleHeight

	Label {
		id: lblMain
		text: showCoaches ? qsTr("Coaches or Trainers") : qsTr("Clients")
		color: AppSettings.fontColor
		font.bold: true
		font.pointSize: AppSettings.fontSizeTitle
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			topMargin: 20
			left: parent.left
			right: parent.right
		}
	}

	Row {
		id: controlsRow
		spacing: 10
		padding: 5
		height: 30

		anchors {
			top: lblMain.bottom
			topMargin: 20
			left: parent.left
			leftMargin: (windowWidth - controlsRow.childrenRect.width)/2
			right: parent.right
		}

		TPButton {
			id: btnAdd
			imageSource: "qrc:/images/add-new.png"
			width: 30
			height: 30

			onClicked: curUserRow = userModel.addUser(showCoaches);
		}

		TPButton {
			id: btnDel
			imageSource: "qrc:/images/remove.png"
			width: 30
			height: 30

			onClicked: showRemoveMessage();
		}

		TPButton {
			id: btnFirst
			imageSource: "qrc:/images/first.png"
			width: 30
			height: 30

			onClicked: curUserRow = userModel.findFirstUser(showCoaches);
		}

		TPButton {
			id: btnPrev
			imageSource: "qrc:/images/prev.png"
			width: 30
			height: 30

			onClicked: curUserRow = userModel.findPrevUser(showCoaches);
		}

		TPButton {
			id: btnNext
			imageSource: "qrc:/images/next.png"
			width: 30
			height: 30

			onClicked: curUserRow = userModel.findNextUser(showCoaches);
		}

		TPButton {
			id: btnLast
			imageSource: "qrc:/images/last.png"
			width: 30
			height: 30

			onClicked: curUserRow = userModel.findLastUser(showCoaches);
		}
	}

	TPCheckBox {
		id: chkCurrent
		text: showCoaches ? qsTr("Default Coach/Trainer") : qsTr("Default Client")
		height: 25

		anchors {
			top: controlsRow.bottom
			topMargin: 10
			left: parent.left
			right: parent.right
		}

		onClicked: {
			if (showCoaches)
				userModel.setCurrentCoach(curUserRow, checked ? curUserRow : 0);
			else
				userModel.setCurrentUser(curUserRow, checked ? curUserRow : 0);
		}

		Component.onCompleted: {
			if (showCoaches)
				checked = userModel.currentCoach(curUserRow) === curUserRow;
			else
				checked = userModel.currentUser(curUserRow) === curUserRow;
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

			UserPersonalData {
				id: usrData
				userRow: curUserRow
				parentPage: coachesOrClientsPage
				width: windowWidth - 20
				height: moduleHeight
			}

			UserContact {
				id: usrContact
				userRow: curUserRow
				width: windowWidth - 20
				Layout.topMargin: -30
			}

			UserProfile {
				id: usrProfile
				userRow: curUserRow
				parentPage: coachesOrClientsPage
				height: moduleHeight
				width: windowWidth - 20
				Layout.topMargin: 20
			}
		}
	}

	footer: ToolBar {
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
			text: qsTr("Save")
			enabled: userModel.modified
			width: 80
			flat: false
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter

			onClicked: appDB.saveUser(curUserRow);
		}
	}

	property TPBalloonTip msgRemoveUser: null
	function showRemoveMessage() {
		if (!AppSettings.alwaysAskConfirmation) {
			curUserRow = appDB.removeUser(curUserRow, showCoaches);
			return;
		}

		if (msgRemoveUser === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveUser = component.createObject(coachesOrClientsPage, { parentPage: coachesOrClientsPage, imageSource: "remove.png",
						message: qsTr("This action cannot be undone."), button1Text: qsTr("Yes"), button2Text: qsTr("No") } );
					msgRemoveUser.button1Clicked.connect(function () { curUserRow = appDB.removeUser(curUserRow, showCoaches); } );
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
