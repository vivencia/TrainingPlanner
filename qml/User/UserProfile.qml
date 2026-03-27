pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Pages

ColumnLayout {
	id: profileModule

//public:
	required property int userRow
	required property TPPage parentPage
	property bool bReady: _clientrole_ok && _coachrole_ok && _goal_ok

//private:
	property bool _clientrole_ok
	property bool _goal_ok
	property bool _coachrole_ok
	property bool _is_client
	property bool _is_coach

	Connections {
		target: AppUserModel
		function onUserModified(row: int, field: int): void {
			if (row === profileModule.userRow) {
				if (field === 20)
					imgAvatar.source = AppUserModel.avatar(profileModule.userRow, false);
				else
					profileModule.getUserInfo();
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
		text: AppUserModel.userRoleLabel
		visible: profileModule._is_client
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 0 : -5
	}

	TPComboBox {
		id: cboUserRole
		model: userRoleModel
		visible: profileModule._is_client
		Layout.fillWidth: true

		onActivated: (index) => {
			if (index < userRoleModel.count - 1) {
				AppUserModel.setUserRole(profileModule.userRow, textAt(index));
				profileModule._clientrole_ok = true;
			}
			else {
				profileModule._clientrole_ok = false;
				txtUserRole.forceActiveFocus();
			}
		}
	}

	TPTextInput {
		id: txtUserRole
		visible: profileModule._is_client && cboUserRole.currentIndex === userRoleModel.count - 1
		heightAdjustable: false
		readOnly: profileModule.userRow !== 0
		Layout.fillWidth: true

		onTextEdited: profileModule._clientrole_ok = text.length > 1
		onEditingFinished: {
			if (profileModule._clientrole_ok)
				AppUserModel.setUserRole(profileModule.userRow, text);
		}
	}

	TPLabel {
		id: lblGoal
		text: AppUserModel.goalLabel
		visible: profileModule._is_client
	}

	TPComboBox {
		id: cboGoal
		model: userGoalModel
		visible: profileModule._is_client
		enabled: profileModule._clientrole_ok
		Layout.fillWidth: true

		onActivated: (index) => {
			if (index < userGoalModel.count - 1) {
				AppUserModel.setGoal(profileModule.userRow, textAt(index));
				profileModule._goal_ok = true;
			}
			else {
				profileModule._goal_ok = false;
				txtUserGoal.forceActiveFocus();
			}
		}
	}

	TPTextInput {
		id: txtUserGoal
		visible: profileModule._is_client && cboGoal.currentIndex === userGoalModel.count - 1
		heightAdjustable: false
		readOnly: profileModule.userRow !== 0
		Layout.fillWidth: true

		onTextEdited: profileModule._goal_ok = text.length > 1
		onEditingFinished: {
			if (profileModule._goal_ok)
				AppUserModel.setGoal(profileModule.userRow, text);
		}
	}

	TPLabel {
		id: lblCoachRole
		text: AppUserModel.coachRoleLabel
		visible: profileModule._is_coach
	}

	TPComboBox {
		id: cboCoachRole
		model: coachRoleModel
		visible: profileModule._is_coach
		enabled: profileModule._clientrole_ok && profileModule._goal_ok
		Layout.fillWidth: true

		onActivated: (index) => {
			if (index < coachRoleModel.count - 1) {
				AppUserModel.setCoachRole(profileModule.userRow, textAt(index));
				profileModule._coachrole_ok = true;
			}
			else {
				profileModule._coachrole_ok = false;
				txtCoachRole.forceActiveFocus();
			}
		}
	}

	TPTextInput {
		id: txtCoachRole
		visible: profileModule._is_coach && cboCoachRole.currentIndex === coachRoleModel.count - 1
		heightAdjustable: false
		readOnly: profileModule.userRow !== 0
		Layout.fillWidth: true

		onTextEdited: profileModule._coachrole_ok = text.length > 1
		onEditingFinished: {
			if (profileModule._coachrole_ok)
				AppUserModel.setCoachRole(profileModule.userRow, text);
		}
	}

	TPLabel {
		id: lblAvatar
		text: AppUserModel.avatarLabel
		color: AppSettings.fontColor
	}

	TPImage {
		id: imgAvatar
		enabled: profileModule.bReady
		Layout.minimumWidth: side_size
		Layout.maximumWidth: side_size
		Layout.minimumHeight: side_size
		Layout.maximumHeight: side_size
		Layout.alignment: Qt.AlignCenter
		Layout.topMargin: -lblAvatar.height

		readonly property int side_size: AppSettings.itemDefaultHeight * 4

		MouseArea {
			enabled: profileModule.userRow === 0
			anchors.fill: parent
			onClicked: profileModule.showAvatarsPopup();
		}
	}

	Loader {
		id: chooseAvatarDlgLoader
		asynchronous: true
		active: false

		property AvatarsPopup _popup

		sourceComponent: AvatarsPopup {
			userRow: profileModule.userRow
			parentPage: profileModule.parentPage
			onAvatarSelected: (id, from_file) => {
				AppUserModel.setAvatar(profileModule.userRow, !from_file ? "image://tpimageprovider/" + id : id);
				imgAvatar.source = AppUserModel.avatar(profileModule.userRow);
			}
			onClosed: chooseAvatarDlgLoader.active = false;
			Component.onCompleted: chooseAvatarDlgLoader._popup = this;
		}

		onLoaded: _popup.showInWindow(-Qt.AlignCenter);
	}
	function showAvatarsPopup(): void {
		chooseAvatarDlgLoader.active = true;
	}

	function defaultAvatarChanged(row: int): void {
		if (row === profileModule.userRow)
			imgAvatar.source = AppUserModel.avatar(profileModule.userRow);
	}

	function getUserInfo(): void {
		if (profileModule.userRow === -1)
			return;
		let idx;
		const enabled = profileModule.userRow === 0;
		profileModule._is_client = AppUserModel.isClient(profileModule.userRow);
		profileModule._is_coach = AppUserModel.isCoach(profileModule.userRow);

		if (profileModule._is_client) {
			const client_role = AppUserModel.userRole(profileModule.userRow);
			profileModule._clientrole_ok = client_role.length > 1;
			if (!profileModule._clientrole_ok)
				cboUserRole.currentIndex = -1;
			else {
				idx = cboUserRole.find(client_role);
				if (idx < 0) {
					idx = userRoleModel.count - 1;
					txtUserRole.text = client_role;
				}
				cboUserRole.currentIndex = idx;
			}

			const user_goal = AppUserModel.goal(profileModule.userRow);
			profileModule._goal_ok = user_goal.length > 1;
			if (!profileModule._goal_ok)
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
			profileModule._goal_ok = profileModule._clientrole_ok = true;

		if (profileModule._is_coach) {
			const coach_role = AppUserModel.coachRole(profileModule.userRow);
			profileModule._coachrole_ok = coach_role.length > 1;
			if (!profileModule._coachrole_ok)
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
			profileModule._coachrole_ok = true;

		imgAvatar.source = AppUserModel.avatar(profileModule.userRow);
	}

	function focusOnFirstField(): void {
		cboUserRole.forceActiveFocus();
	}
}
