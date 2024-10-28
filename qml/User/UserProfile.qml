import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Pages"

Frame {
	id: frmUserProfile
	spacing: 5
	padding: 0
	height: minimumHeight
	implicitWidth: width
	implicitHeight: height

	required property int userRow
	property bool bReady: (bClientRoleOK || bCoachRoleOK) & bGoalOK
	property bool bCoachRoleOK: false
	property bool bClientRoleOK: false
	property bool bGoalOK: false
	readonly property int controlsHeight: 25
	readonly property int controlsSpacing: 10
	readonly property int minimumHeight: 7*controlsHeight + imgAvatar.height

	required property TPPage parentPage

	ListModel {
		id: roleModelUser
		ListElement { text: qsTr("Occasional Gym Goer"); value: 0; enabled: true; }
		ListElement { text: qsTr("Serious Gym Goer"); value: 1; enabled: true; }
		ListElement { text: qsTr("Aspiring Athlete"); value: 2; enabled: true; }
		ListElement { text: qsTr("Amateur Athlete"); value: 3; enabled: true; }
		ListElement { text: qsTr("Professional Athlete"); value: 4; enabled: true; }
		ListElement { text: qsTr("Other"); value: 5; enabled: true; }
	}

	ListModel {
		id: roleModelCoach
		ListElement { text: qsTr("Personal Trainer"); value: 0; enabled: true; }
		ListElement { text: qsTr("Athletes coach"); value: 1; enabled: true; }
		ListElement { text: qsTr("Physical Therapist"); value: 2; enabled: true; }
		ListElement { text: qsTr("Other"); value: 5; enabled: true; }
	}

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	onUserRowChanged: {
		cboUserRole.currentIndex = cboUserRole.find(userModel.userRole(userRow));
		bClientRoleOK = userModel.userRole(userRow).length > 1;
		cboGoal.currentIndex = cboGoal.find(userModel.goal(userRow));
		bGoalOK = userModel.goal(userRow).length > 1;
		cboCoachRole.currentIndex = cboCoachRole.find(userModel.coachRole(userRow));
		bCoachRoleOK = userModel.coachRole(userRow).length > 1;
	}

	TPLabel {
		id: lblUserRole
		text: userModel.userRoleLabel
		visible: userModel.appUseMode(userRow) !== 2
		height: controlsHeight
		width: parent.width*0.20

		anchors {
			top: parent.top
			topMargin: -5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPComboBox {
		id: cboUserRole
		model: roleModelUser
		visible: lblUserRole.visible
		height: controlsHeight
		width: parent.width*0.80

		anchors {
			top: lblUserRole.bottom
			topMargin: -5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onActivated: (index) => {
			userModel.setUserRole(userRow, textAt(index));
			bClientRoleOK = true;
		}

		Component.onCompleted: {
			currentIndex = find(userModel.userRole(userRow));
			bClientRoleOK = userModel.userRole(userRow).length > 1;
		}
	}

	TPLabel {
		id: lblGoal
		text: userModel.goalLabel
		visible: userModel.appUseMode(userRow) !== 2
		height: controlsHeight
		width: parent.width*0.20

		anchors {
			top: cboUserRole.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPComboBox {
		id: cboGoal
		model: goalModel
		visible: lblGoal.visible
		enabled: bClientRoleOK || bCoachRoleOK
		height: controlsHeight
		width: parent.width*0.80

		ListModel {
			id: goalModel
			ListElement { text: qsTr("General Fitness"); value: 0; enabled: true; }
			ListElement { text: qsTr("Loose Weight"); value: 1; enabled: true; }
			ListElement { text: qsTr("Improve Health"); value: 2; enabled: true; }
			ListElement { text: qsTr("Support for Other Sport"); value: 3; enabled: true; }
			ListElement { text: qsTr("Muscle Gain"); value: 4; enabled: true; }
			ListElement { text: qsTr("Strength"); value: 5; enabled: true; }
			ListElement { text: qsTr("Bodybuilding"); value: 6; enabled: true; }
			ListElement { text: qsTr("Other"); value: 7; enabled: true; }
		}

		anchors {
			top: lblGoal.bottom
			topMargin: -5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onActivated: (index) => {
			userModel.setGoal(userRow, textAt(index));
			bGoalOK = true;
		}

		Component.onCompleted: {
			currentIndex = find(userModel.goal(userRow));
			bGoalOK = userModel.goal(userRow).length > 1;
		}
	}

	TPLabel {
		id: lblCoachRole
		text: userModel.coachRoleLabel
		visible: userModel.appUseMode(userRow) === 2 || userModel.appUseMode(userRow) === 4
		height: controlsHeight
		width: parent.width*0.15

		anchors {
			top: cboGoal.visible ? cboGoal.bottom : cboUserRole.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPComboBox {
		id: cboCoachRole
		model: roleModelCoach
		visible: lblCoachRole.visible
		height: controlsHeight
		width: parent.width*0.80

		anchors {
			top: lblCoachRole.bottom
			topMargin: -5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onActivated: (index) => {
			userModel.setCoachRole(userRow, textAt(index));
			bCoachRoleOK = true;
		}

		Component.onCompleted: {
			currentIndex = find(userModel.coachRole(userRow));
			bCoachRoleOK = userModel.coachRole(userRow).length > 1;
		}
	}

	TPLabel {
		id: lblAvatar
		text: userModel.avatarLabel
		color: appSettings.fontColor
		height: controlsHeight
		width: parent.width*0.2

		anchors {
			top: cboCoachRole.visible ? cboCoachRole.bottom : cboGoal.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
		}
	}

	TPImage {
		id: imgAvatar
		source: userModel.avatar(userRow)
		height: 100
		width: 100

		anchors {
			top: lblAvatar.top
			topMargin: 0
			left: lblAvatar.right
			leftMargin: 30
		}

		MouseArea {
			anchors.fill: parent
			onClicked: showAvatarsPopup();
		}
	}

	Component.onCompleted: {
		userModel.updateGUI.connect(function () {
			lblUserRole.text = userModel.userRoleLabel;
			lblGoal.text = userModel.goalLabel;
			lblCoachRole.text = userModel.coachRoleLabel;
			lblAvatar.text = userModel.avatarLabel;
		});
	}

	property AvatarsPopup chooseAvatarDlg: null
	function showAvatarsPopup() {
		if (chooseAvatarDlg === null) {
			function createAvatarsDialog() {
				var component = Qt.createComponent("qrc:/qml/User/AvatarsPopup.qml", Qt.Asynchronous);

				function finishCreation() {
					chooseAvatarDlg = component.createObject(parentPage, { userRow: frmUserProfile.userRow, parentPage: parentPage, callerWidget: frmUserProfile });
					chooseAvatarDlg.open();
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createAvatarsDialog();
		}
		chooseAvatarDlg.open();
	}

	function selectAvatar(id: string) {
		userModel.setAvatar(userRow, "image://tpimageprovider/" + id);
		imgAvatar.source = userModel.avatar(userRow);
	}

	function selectExternalAvatar(filename: string) {
		userModel.setAvatar(userRow, filename);
		imgAvatar.source = userModel.avatar(userRow);
	}

	function focusOnFirstField() {
		return;
	}
}
