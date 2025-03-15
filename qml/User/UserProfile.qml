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
	property bool bClientRoleOK: appUseMode !== 2
	property bool bGoalOK: appUseMode !== 2
	property bool bCoachRoleOK: appUseMode === 2 || appUseMode === 4
	property int appUseMode
	readonly property int nVisibleControls: lblCoachRole.visible ? 9 : 7
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
		id: userRoleModel
		ListElement { text: qsTr("Occasional Gym Goer"); value: 0; enabled: true; }
		ListElement { text: qsTr("Serious Gym Goer"); value: 1; enabled: true; }
		ListElement { text: qsTr("Aspiring Athlete"); value: 2; enabled: true; }
		ListElement { text: qsTr("Amateur Athlete"); value: 3; enabled: true; }
		ListElement { text: qsTr("Professional Athlete"); value: 4; enabled: true; }
		ListElement { text: qsTr("Other"); value: 5; enabled: true; }
	}

	ListModel {
		id: coachRoleModel
		ListElement { text: qsTr("Personal Trainer"); value: 0; enabled: true; }
		ListElement { text: qsTr("Athletes coach"); value: 1; enabled: true; }
		ListElement { text: qsTr("Physical Therapist"); value: 2; enabled: true; }
		ListElement { text: qsTr("Other"); value: 5; enabled: true; }
	}

	ListModel {
		id: userGoalModel
		ListElement { text: qsTr("General Fitness"); value: 0; enabled: true; }
		ListElement { text: qsTr("Loose Weight"); value: 1; enabled: true; }
		ListElement { text: qsTr("Improve Health"); value: 2; enabled: true; }
		ListElement { text: qsTr("Support for Other Sport"); value: 3; enabled: true; }
		ListElement { text: qsTr("Muscle Gain"); value: 4; enabled: true; }
		ListElement { text: qsTr("Strength"); value: 5; enabled: true; }
		ListElement { text: qsTr("Bodybuilding"); value: 6; enabled: true; }
		ListElement { text: qsTr("Other"); value: 7; enabled: true; }
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
		model: userRoleModel
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
			if (index < userRoleModel.count - 1) {
				userModel.setUserRole(userRow, textAt(index));
				bClientRoleOK = true;
			}
			else {
				bClientRoleOK = false;
				txtUserRole.forceActiveFocus();
			}
		}
	}

	TPTextInput {
		id: txtUserRole
		visible: cboUserRole.currentIndex === userRoleModel.count - 1
		height: controlsHeight

		anchors {
			top: cboUserRole.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onTextEdited: bClientRoleOK = text.length > 1

		onEditingFinished: {
			if (bClientRoleOK)
				userModel.setUserRole(userRow, text);
		}
	}

	TPLabel {
		id: lblGoal
		text: userModel.goalLabel
		visible: appUseMode !== 2
		height: controlsHeight
		width: parent.width*0.20

		anchors {
			top: txtUserRole.visible ? txtUserRole.bottom : cboUserRole.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPComboBox {
		id: cboGoal
		model: userGoalModel
		visible: appUseMode !== 2
		enabled: bClientRoleOK
		height: controlsHeight
		width: parent.width*0.80

		anchors {
			top: lblGoal.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onActivated: (index) => {
			if (index < userGoalModel.count - 1) {
				userModel.setGoal(userRow, textAt(index));
				bGoalOK = true;
			}
			else {
				bGoalOK = false;
				txtUserGoal.forceActiveFocus();
			}
		}
	}

	TPTextInput {
		id: txtUserGoal
		visible: cboGoal.visible && cboGoal.currentIndex === userGoalModel.count - 1
		height: controlsHeight

		anchors {
			top: cboGoal.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onTextEdited: bGoalOK = text.length > 1

		onEditingFinished: {
			if (bGoalOK)
				userModel.setGoal(userRow, text);
		}
	}

	TPLabel {
		id: lblCoachRole
		text: userModel.coachRoleLabel
		visible: appUseMode === 2 || appUseMode === 4
		height: controlsHeight
		width: parent.width*0.15

		anchors {
			top: cboGoal.visible ? txtUserGoal.visible ? txtUserGoal.bottom : cboGoal.bottom : cboUserRole.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPComboBox {
		id: cboCoachRole
		model: coachRoleModel
		visible: appUseMode === 2 || appUseMode === 4
		enabled: bClientRoleOK && bGoalOK
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
			if (index < coachRoleModel.count - 1) {
				userModel.setCoachRole(userRow, textAt(index));
				bCoachRoleOK = true;
			}
			else {
				bCoachRoleOK = false;
				txtCoachRole.forceActiveFocus();
			}
		}
	}

	TPTextInput {
		id: txtCoachRole
		visible: cboCoachRole.visible && cboCoachRole.currentIndex === coachRoleModel.count - 1
		height: controlsHeight

		anchors {
			top: cboCoachRole.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onTextEdited: bCoachRoleOK = text.length > 1

		onEditingFinished: {
			if (bCoachRoleOK)
				userModel.setCoachRole(userRow, text);
		}
	}

	TPLabel {
		id: lblAvatar
		text: userModel.avatarLabel
		color: appSettings.fontColor
		height: controlsHeight
		width: parent.width*0.2

		anchors {
			top: cboCoachRole.visible ? txtCoachRole.visible ? txtCoachRole.bottom : cboCoachRole.bottom : cboGoal.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
		}
	}

	TPImage {
		id: imgAvatar
		enabled: bReady
		height: 100
		width: 100

		anchors {
			top: lblAvatar.top
			topMargin: 0
			left: lblAvatar.right
			leftMargin: 30
		}

		MouseArea {
			enabled: userRow === 0
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

	function makeModelsSelectable(): void {
		const enabled = userRow === 0;
		for (let i = 0; i < userRoleModel.count; ++i)
			userRoleModel.get(i).enabled = enabled;
		for (let x = 0; x < coachRoleModel.count; ++x)
			coachRoleModel.get(x).enabled = enabled;
		for (let y = 0; y < userGoalModel.count; ++y)
			userGoalModel.get(y).enabled = enabled;
	}

	function getUserInfo(): void {
		if (userRow === -1)
			return;
		let idx;
		appUseMode = userModel.appUseMode(userRow);

		const client_role = userModel.userRole(userRow);
		bClientRoleOK = client_role.length > 1;
		if (!bClientRoleOK)
			cboUserRole.currentIndex = 0;
		else {
			idx = cboUserRole.find(client_role);
			if (idx < 0) {
				idx = userRoleModel.count - 1;
				txtUserRole.text = client_role;
			}
			cboUserRole.currentIndex = idx;
		}

		const user_goal = userModel.goal(userRow);
		bGoalOK = user_goal.length > 1;
		if (!bGoalOK)
			cboGoal.currentIndex = 0;
		else {
			idx = cboGoal.find(user_goal);
			if (idx < 0) {
				idx = userGoalModel.count - 1;
				txtUserGoal.text = user_goal;
			}
			cboGoal.currentIndex = idx;
		}

		const coach_role = userModel.coachRole(userRow);
		bCoachRoleOK = coach_role.length > 1;
		if (!bCoachRoleOK)
			cboCoachRole.currentIndex = 0;
		else {
			idx = cboCoachRole.find(coach_role);
			if (idx < 0) {
				idx = coachRoleModel.count - 1;
				txtCoachRole.text = coach_role;
			}
			cboCoachRole.currentIndex = idx;
		}

		const avatar_src = userModel.avatar(userRow);
		if (avatar_src !== "")
			imgAvatar.source = userModel.avatar(userRow);
		else
			imgAvatar.source = userModel.defaultAvatar(userRow);
		makeModelsSelectable();
	}

	function focusOnFirstField(): void {
		return;
	}
}
