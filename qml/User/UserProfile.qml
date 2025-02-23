import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Pages"

Frame {
	id: frmUserProfile
	padding: 0
	spacing: 0
	height: moduleHeight
	implicitHeight: Math.min(height, moduleHeight)
	implicitWidth: width

	required property int userRow
	required property TPPage parentPage
	property bool bReady: (bClientRoleOK || bCoachRoleOK) & bGoalOK
	property bool bCoachRoleOK: false
	property bool bClientRoleOK: false
	property bool bGoalOK: false
	property int appUseMode
	readonly property int nVisibleControls: lblCoachRole.visible ? 6 : 4
	readonly property int controlsHeight: 25
	readonly property int controlsSpacing: 10
	readonly property int moduleHeight: nVisibleControls*(controlsHeight+controlsSpacing) + imgAvatar.height

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === userRow) {
				switch (field) {
					case 100: getUserInfo(); break;
					case 20: imgAvatar.source = userModel.avatar(userRow); break;
					case 10: appUseMode = userModel.appUseMode(userRow); break;
					default: break;
				}
			}
		}
	}

	onUserRowChanged: getUserInfo();
	Component.onCompleted: getUserInfo();

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

	TPLabel {
		id: lblUserRole
		text: userModel.userRoleLabel
		visible: appUseMode !== 2
		height: controlsHeight
		width: parent.width*0.20

		anchors {
			top: parent.top
			topMargin: -10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPComboBox {
		id: cboUserRole
		model: roleModelUser
		visible: appUseMode !== 2
		height: controlsHeight
		width: parent.width*0.80

		anchors {
			top: lblUserRole.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onActivated: (index) => {
			userModel.setUserRole(userRow, textAt(index));
			bClientRoleOK = true;
		}
	}

	TPLabel {
		id: lblGoal
		text: userModel.goalLabel
		visible: appUseMode !== 2
		height: controlsHeight
		width: parent.width*0.20

		anchors {
			top: cboUserRole.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPComboBox {
		id: cboGoal
		model: goalModel
		visible: appUseMode !== 2
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
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onActivated: (index) => {
			userModel.setGoal(userRow, textAt(index));
			bGoalOK = true;
		}
	}

	TPLabel {
		id: lblCoachRole
		text: userModel.coachRoleLabel
		visible: appUseMode === 2 || appUseMode === 4
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
		visible: appUseMode === 2 || appUseMode === 4
		height: controlsHeight
		width: parent.width*0.80

		anchors {
			top: lblCoachRole.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onActivated: (index) => {
			userModel.setCoachRole(userRow, textAt(index));
			bCoachRoleOK = true;
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

	property AvatarsPopup chooseAvatarDlg: null
	function showAvatarsPopup(): void {
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

	function selectAvatar(id: string): void {
		userModel.setAvatar(userRow, "image://tpimageprovider/" + id);
		imgAvatar.source = userModel.avatar(userRow);
	}

	function selectExternalAvatar(filename: string): void {
		userModel.setAvatar(userRow, filename);
		imgAvatar.source = userModel.avatar(userRow);
	}

	function defaultAvatarChanged(row: int): void {
		if (row === userRow)
			imgAvatar.source = userModel.avatar(userRow);
	}

	function getUserInfo(): void {
		appUseMode = userModel.appUseMode(userRow);
		const client_role = userModel.userRole(userRow);
		bClientRoleOK = client_role.length > 5;
		cboUserRole.currentIndex = bClientRoleOK ? cboUserRole.find(client_role) : -1;
		const user_goal = userModel.goal(userRow);
		bGoalOK = user_goal.length > 1;
		cboGoal.currentIndex = bGoalOK ? cboGoal.find(user_goal) : -1;
		const coach_role = userModel.coachRole(userRow);
		bCoachRoleOK = coach_role.length > 5;
		cboCoachRole.currentIndex = bCoachRoleOK ? cboCoachRole.find(coach_role) : -1;
		imgAvatar.source = userModel.avatar(userRow);
	}

	function focusOnFirstField(): void {
		return;
	}
}
