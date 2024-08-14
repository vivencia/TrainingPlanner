import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import com.vivenciasoftware.qmlcomponents

import ".."
import "../TPWidgets"
import "../Pages"

Frame {
	id: frmUserProfile
	implicitWidth: width
	implicitHeight: allControlsHeight + nControls*controlsSpacing
	spacing: 5
	padding: 0

	ListModel {
		id: roleModelUser
		ListElement { text: qsTr("Occasional Gym Goer"); value: 0; }
		ListElement { text: qsTr("Serious Gym Goer"); value: 1; }
		ListElement { text: qsTr("Aspiring Athlete"); value: 2; }
		ListElement { text: qsTr("Amateur Athlete"); value: 3; }
		ListElement { text: qsTr("Professional Athlete"); value: 4; }
		ListElement { text: qsTr("Other"); value: 5; }
	}

	ListModel {
		id: roleModelCoach
		ListElement { text: qsTr("Personal Trainer"); value: 0; }
		ListElement { text: qsTr("Athletes coach"); value: 1; }
		ListElement { text: qsTr("Physical Therapist"); value: 2; }
		ListElement { text: qsTr("Other"); value: 5; }
	}

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	property int allControlsHeight: nControls*controlsHeight
	property bool bReady: bRoleOK & bGoalOK
	property bool bRoleOK: false
	property bool bGoalOK: false
	readonly property int nControls: 4
	readonly property int controlsHeight: 25
	readonly property int controlsSpacing: 10
	required property TPPage parentPage

	Label {
		id: lblUserRole
		text: userModel.columnLabel(7)
		color: AppSettings.fontColor
		visible: userModel.appUseMode !== 2
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		bottomPadding: 0
		width: parent.width*0.2

		onHeightChanged: {
			if (visible)
				allControlsHeight += height;
			else
				allControlsHeight -= height;
		}

		anchors {
			top: parent.top
			left: parent.left
			leftMargin: 5
		}
	}

	TPComboBox {
		id: cboUserRole
		model: roleModelUser
		visible: userModel.appUseMode !== 2
		height: controlsHeight
		width: parent.width*0.80

		Component.onCompleted: {
			currentIndex = find(userModel.role);
			bRoleOK = !userModel.isEmpty();
		}

		onActivated: (index) => {
			userModel.role = textAt(index);
			bRoleOK = true;
		}

		anchors {
			top: parent.top
			topMargin: -5
			left: lblUserRole.right
			leftMargin: 0
		}
	}

	Label {
		id: lblGoal
		text: userModel.columnLabel(8)
		visible: userModel.appUseMode !== 2
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		width: parent.width*0.2

		onHeightChanged: {
			if (visible)
				allControlsHeight += height;
			else
				allControlsHeight -= height;
		}

		anchors {
			top: lblUserRole.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
		}
	}

	TPComboBox {
		id: cboGoal
		model: goalModel
		visible: userModel.appUseMode !== 2
		enabled: bRoleOK
		height: controlsHeight
		width: parent.width*0.80

		ListModel {
			id: goalModel
			ListElement { text: qsTr("General Fitness"); value: 0; }
			ListElement { text: qsTr("Loose Weight"); value: 1; }
			ListElement { text: qsTr("Improve Health"); value: 2; }
			ListElement { text: qsTr("Support for Other Sport"); value: 3; }
			ListElement { text: qsTr("Muscle Gain"); value: 4; }
			ListElement { text: qsTr("Strength"); value: 5; }
			ListElement { text: qsTr("Bodybuilding"); value: 6; }
			ListElement { text: qsTr("Other"); value: 7; }
		}

		Component.onCompleted: {
			currentIndex = find(userModel.goal);
			bGoalOK = !userModel.isEmpty();
		}

		onActivated: (index) => {
			userModel.goal = textAt(index);
			bGoalOK = true;
		}

		anchors {
			top: lblGoal.top
			topMargin: -5
			left: lblGoal.right
			leftMargin: 0
		}
	}

	Label {
		id: lblCoachRole
		text: userModel.columnLabel(7)
		color: AppSettings.fontColor
		visible: userModel.appUseMode === 2 || userModel.appUseMode === 4
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		bottomPadding: 0
		width: parent.width*0.2

		onHeightChanged: {
			if (visible)
				allControlsHeight += height;
			else
				allControlsHeight -= height;
		}

		anchors {
			top: lblGoal.visible ? lblGoal.bottom : parent.top
			left: parent.left
			leftMargin: 5
		}
	}

	TPComboBox {
		id: cboCoachRole
		model: roleModelCoach
		visible: userModel.appUseMode === 2 || userModel.appUseMode === 4
		height: controlsHeight
		width: parent.width*0.80

		Component.onCompleted: {
			currentIndex = find(userModel.coachRole);
			bRoleOK = !userModel.isEmpty();
		}

		onActivated: (index) => {
			userModel.coachRole = textAt(index);
			bRoleOK = true;
		}

		anchors {
			top: lblCoachRole.top
			topMargin: -5
			left: lblCoachRole.right
			leftMargin: 0
		}
	}

	Label {
		id: lblAvatar
		text: qsTr("Avatar:")
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		padding: 0
		height: controlsHeight
		width: parent.width*0.2

		anchors {
			top: lblGoal.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
		}
	}

	Rectangle {
		id: recAvatar
		height: 4*controlsHeight
		width: height
		layer.enabled: true
		visible: false
		color: "transparent"

		Component.onCompleted: allControlsHeight += height;

		Image {
			anchors.fill: parent
			source: userModel.avatar
		}

		anchors {
			top: lblAvatar.top
			topMargin: -10
			left: lblAvatar.right
			leftMargin: 30
		}
	}

	MultiEffect {
		id: recAvatarEffect
		visible: true
		source: recAvatar
		anchors.fill: recAvatar
		shadowEnabled: true
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1

		MouseArea {
			anchors.fill: parent
			onClicked: showAvatarsPopup();
		}
	}

	property AvatarsPopup chooseAvatarDlg: null
	function showAvatarsPopup() {
		if (chooseAvatarDlg === null) {
			function createAvatarsDialog() {
				var component = Qt.createComponent("qrc:/qml/User/AvatarsPopup.qml", Qt.Asynchronous);

				function finishCreation() {
					chooseAvatarDlg = component.createObject(parentPage, { parentPage: parentPage,
							targetImageItem: recAvatarEffect, callerWidget: frmUserProfile });
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

	function selectAvatar(id: int) {
		userModel.avatar = "image://tpimageprovider/" + parseInt(id);
	}
	function selectExternalAvatar(filename: string) {
		userModel.avatar = filename;
	}

	function focusOnFirstField() {
		return;
	}
}
