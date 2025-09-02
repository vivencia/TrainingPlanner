import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Pages"

ColumnLayout {
	id: profileModule

	required property int userRow
	required property TPPage parentPage
	property bool bReady: bClientRoleOK && bCoachRoleOK && bGoalOK
	property bool bClientRoleOK
	property bool bGoalOK
	property bool bCoachRoleOK
	property bool isClient
	property bool isCoach

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === userRow) {
				if (field >= 100)
					getUserInfo();
				else if (field === 20)
					imgAvatar.source = userModel.avatar(userRow, false);
				else if (field === 11) {
					isClient = userModel.isClient(userRow);
					isCoach = userModel.isCoach(userRow);
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
		ListElement { text: qsTr("Physical Educator"); value: 3; enabled: true; }
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

	TPLabel {
		id: lblUserRole
		text: userModel.userRoleLabel
		visible: isClient
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 0 : -5
	}

	TPComboBox {
		id: cboUserRole
		model: userRoleModel
		visible: isClient
		Layout.fillWidth: true

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
		visible: isClient && cboUserRole.currentIndex === userRoleModel.count - 1
		heightAdjustable: false
		readOnly: userRow !== 0
		Layout.fillWidth: true

		onTextEdited: bClientRoleOK = text.length > 1
		onEditingFinished: {
			if (bClientRoleOK)
				userModel.setUserRole(userRow, text);
		}
	}

	TPLabel {
		id: lblGoal
		text: userModel.goalLabel
		visible: isClient
	}

	TPComboBox {
		id: cboGoal
		model: userGoalModel
		visible: isClient
		enabled: bClientRoleOK
		Layout.fillWidth: true

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
		visible: isClient && cboGoal.currentIndex === userGoalModel.count - 1
		heightAdjustable: false
		readOnly: userRow !== 0
		Layout.fillWidth: true

		onTextEdited: bGoalOK = text.length > 1
		onEditingFinished: {
			if (bGoalOK)
				userModel.setGoal(userRow, text);
		}
	}

	TPLabel {
		id: lblCoachRole
		text: userModel.coachRoleLabel
		visible: isCoach
	}

	TPComboBox {
		id: cboCoachRole
		model: coachRoleModel
		visible: isCoach
		enabled: bClientRoleOK && bGoalOK
		Layout.fillWidth: true

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
		visible: isCoach && cboCoachRole.currentIndex === coachRoleModel.count - 1
		heightAdjustable: false
		readOnly: userRow !== 0
		Layout.fillWidth: true

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
	}

	TPImage {
		id: imgAvatar
		enabled: bReady
		Layout.minimumWidth: side_size
		Layout.maximumWidth: side_size
		Layout.minimumHeight: side_size
		Layout.maximumHeight: side_size
		Layout.alignment: Qt.AlignCenter
		Layout.topMargin: -lblAvatar.height

		readonly property int side_size: appSettings.itemDefaultHeight*4

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
				let component = Qt.createComponent("qrc:/qml/User/AvatarsPopup.qml", Qt.Asynchronous);

				function finishCreation() {
					chooseAvatarDlg = component.createObject(parentPage, { userRow: profileModule.userRow,
										parentPage: parentPage, callerWidget: profileModule });
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
		if (userRow === -1)
			return;
		let idx;
		const enabled = userRow === 0;
		isClient = userModel.isClient(userRow);
		isCoach = userModel.isCoach(userRow);

		if (isClient) {
			const client_role = userModel.userRole(userRow);
			bClientRoleOK = client_role.length > 1;
			if (!bClientRoleOK)
				cboUserRole.currentIndex = -1;
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
				cboGoal.currentIndex = -1;
			else {
				idx = cboGoal.find(user_goal);
				if (idx < 0) {
					idx = userGoalModel.count - 1;
					txtUserGoal.text = user_goal;
				}
				cboGoal.currentIndex = idx;
			}

			for (let i = 0; i < userRoleModel.count; ++i)
				userRoleModel.get(i).enabled = enabled;
			for (let y = 0; y < userGoalModel.count; ++y)
				userGoalModel.get(y).enabled = enabled;
		}
		else
			bGoalOK = bClientRoleOK = true;

		if (isCoach) {
			const coach_role = userModel.coachRole(userRow);
			bCoachRoleOK = coach_role.length > 1;
			if (!bCoachRoleOK)
				cboCoachRole.currentIndex = -1;
			else {
				idx = cboCoachRole.find(coach_role);
				if (idx < 0) {
					idx = coachRoleModel.count - 1;
					txtCoachRole.text = coach_role;
				}
				cboCoachRole.currentIndex = idx;
			}
			for (let x = 0; x < coachRoleModel.count; ++x)
				coachRoleModel.get(x).enabled = enabled;
		}
		else
			bCoachRoleOK = true;

		imgAvatar.source = userModel.avatar(userRow);
	}

	function focusOnFirstField(): void {
		return;
	}
}
