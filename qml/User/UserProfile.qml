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
	implicitHeight: height
	spacing: 5
	padding: 0

	required property int userRow
	property bool bReady: bRoleOK & bGoalOK
	property bool bRoleOK: false
	property bool bGoalOK: false
	readonly property int controlsHeight: 25
	readonly property int controlsSpacing: 10
	required property TPPage parentPage

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

	Label {
		id: lblUserRole
		text: userModel.columnLabel(7)
		color: AppSettings.fontColor
		visible: userModel.appUseMode(userRow) !== 2
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		bottomPadding: 0
		width: parent.width*0.15

		anchors {
			top: parent.top
			left: parent.left
			leftMargin: 5
		}
	}

	TPComboBox {
		id: cboUserRole
		model: roleModelUser
		visible: lblUserRole.visible
		height: controlsHeight
		width: parent.width*0.80

		Component.onCompleted: {
			currentIndex = find(userModel.userRole(userRow));
			bRoleOK = userModel.userRole(userRow).length > 1
		}

		onActivated: (index) => {
			userModel.setUserRole(userRow, textAt(index));
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
		visible: userModel.appUseMode(userRow) !== 2
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		width: parent.width*0.15

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
		visible: lblGoal.visible
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
			currentIndex = find(userModel.goal(userRow));
			bGoalOK = userModel.goal(userRow).length > 1
		}

		onActivated: (index) => {
			userModel.setGoal(userRow, textAt(index));
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
		visible: userModel.appUseMode(userRow) === 2 || userModel.appUseMode(userRow) === 4
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		bottomPadding: 0
		width: parent.width*0.15

		anchors {
			top: lblGoal.visible ? lblGoal.bottom : parent.top
			left: parent.left
			leftMargin: 5
		}
	}

	TPComboBox {
		id: cboCoachRole
		model: roleModelCoach
		visible: lblCoachRole.visible
		height: controlsHeight
		width: parent.width*0.80

		Component.onCompleted: {
			currentIndex = find(userModel.coachRole(userRow));
			bRoleOK = userModel.coachRole(userRow).length > 1
		}

		onActivated: (index) => {
			userModel.setCoachRole(userRow, textAt(index));
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
			top: lblCoachRole.visible ? lblCoachRole.bottom : lblGoal.bottom
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

		Image {
			id: imgAvatar
			anchors.fill: parent
			source: userModel.avatar(userRow)
			asynchronous: true
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
		userModel.setAvatar(userRow, "image://tpimageprovider/" + parseInt(id));
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
