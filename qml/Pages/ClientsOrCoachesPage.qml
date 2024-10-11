import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: clientsOrCoachesPage
	objectName: "clientsOrCoachesPage"

	required property UserManager userManager

	property int curUserRow
	property int firstUserRow
	property int lastUserRow
	property bool showUsers
	property bool showCoaches

	onCurUserRowChanged: {
		if (showCoaches)
			chkCurrent.checked = userModel.currentCoach(0) === curUserRow;
		else
			chkCurrent.checked = userModel.currentUser(0) === curUserRow;
	}

	TPLabel {
		id: lblMain
		text: showCoaches ? qsTr("Coaches or Trainers") : qsTr("Clients")
		font: AppGlobals.titleFont
		width: parent.width
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
			leftMargin: (appSettings.pageWidth - controlsRow.childrenRect.width)/2
			right: parent.right
		}

		TPButton {
			id: btnAdd
			imageSource: "add-new"
			width: 30
			height: 30

			onClicked: {
				const _lastUserRow = userModel.addUser(showCoaches);
				if (_lastUserRow > 0) {
					curUserRow = _lastUserRow;
					lastUserRow = _lastUserRow;
					if (firstUserRow == -1)
						firstUserRow = _lastUserRow;
					usrData.focusOnFirstField();
				}
			}
		}

		TPButton {
			id: btnDel
			imageSource: "remove"
			enabled: curUserRow >= 1
			width: 30
			height: 30

			onClicked: showRemoveMessage();
		}

		TPButton {
			id: btnFirst
			imageSource: "first"
			enabled: curUserRow !== firstUserRow
			width: 30
			height: 30

			onClicked: {
				const _firstUserRow = userModel.findFirstUser(showCoaches);
				if (_firstUserRow > 0) {
					curUserRow = _firstUserRow;
					if (firstUserRow !== _firstUserRow)
						firstUserRow = _firstUserRow;
				}
			}
		}

		TPButton {
			id: btnPrev
			imageSource: "prev"
			enabled: curUserRow !== firstUserRow
			width: 30
			height: 30

			onClicked: {
				const _prevUserRow = userModel.findPrevUser(showCoaches);
				if (_prevUserRow > 0)
					curUserRow = _prevUserRow;
			}
		}

		TPButton {
			id: btnNext
			imageSource: "next"
			enabled: curUserRow !== lastUserRow
			width: 30
			height: 30

			onClicked: {
				const _nextUserRow = userModel.findNextUser(showCoaches);
				if (_nextUserRow > 0)
					curUserRow = _nextUserRow;
			}
		}

		TPButton {
			id: btnLast
			imageSource: "last"
			enabled: curUserRow !== lastUserRow
			width: 30
			height: 30

			onClicked: {
				const _lastUserRow = userModel.findLastUser(showCoaches);
				if (_lastUserRow > 0) {
					curUserRow = _lastUserRow;
					if (_lastUserRow !== _lastUserRow)
						_lastUserRow = _lastUserRow;
				}
			}
		}
	}

	TPCheckBox {
		id: chkCurrent
		text: showCoaches ? qsTr("Default Coach/Trainer") : qsTr("Default Client")
		enabled: curUserRow > 0
		height: 25

		anchors {
			top: controlsRow.bottom
			topMargin: 10
			left: parent.left
			right: parent.right
		}

		onClicked: {
			if (showCoaches)
				userModel.setCurrentCoach(0, checked ? curUserRow : -1);
			else
				userModel.setCurrentUser(0, checked ? curUserRow : -1);
		}
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight

		anchors {
			top: chkCurrent.bottom
			topMargin: 10
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
				parentPage: clientsOrCoachesPage
				enabled: curUserRow > 0
				width: appSettings.pageWidth - 20
			}

			UserContact {
				id: usrContact
				userRow: curUserRow
				enabled: curUserRow > 0
				width: appSettings.pageWidth - 20
				Layout.topMargin: -30
			}

			UserProfile {
				id: usrProfile
				userRow: curUserRow
				parentPage: clientsOrCoachesPage
				enabled: curUserRow > 0
				width: appSettings.pageWidth - 20
				Layout.topMargin: 20
			}
		}
	}

	property TPBalloonTip msgRemoveUser: null
	function showRemoveMessage() {
		if (!appSettings.alwaysAskConfirmation) {
			userManager.removeUser(curUserRow, showCoaches);
			return;
		}

		if (msgRemoveUser === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveUser = component.createObject(clientsOrCoachesPage, { parentPage: clientsOrCoachesPage, imageSource: "remove.png",
						message: qsTr("This action cannot be undone."), button1Text: qsTr("Yes"), button2Text: qsTr("No") } );
					msgRemoveUser.button1Clicked.connect(function () { userManager.removeUser(curUserRow, showCoaches); } );
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
