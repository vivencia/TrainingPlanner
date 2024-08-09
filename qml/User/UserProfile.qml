import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import com.vivenciasoftware.qmlcomponents

import ".."
import "../TPWidgets"

Frame {
	id: frmUserProfile
	property bool bReady: bRoleOK & bGoalOK
	property bool bRoleOK: false
	property bool bGoalOK: false
	readonly property int nControls: 6
	readonly property int controlsHeight: 25
	readonly property int allControlsHeight: nControls*controlsHeight
	readonly property int controlsSpacing: 10

	Label {
		id: lblRole
		text: userModel.columnLabel(7)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		bottomPadding: 0
		width: parent.width*0.2

		anchors {
			top: parent.top
			left: parent.left
			leftMargin: 5
		}
	}

	TPComboBox {
		id: cboRole
		height: controlsHeight
		model: roleModel
		width: parent.width*0.80

		readonly property var roleModel: [ { text:qsTr("Occasional Gym Goer"), value:0 }, { text:qsTr("Serious Gym Goer"), value:1 },
				{ text:qsTr("Aspiring Athlete"), value:2 }, { text:qsTr("Amateur Athlete"), value:3 }, { text:qsTr("Professional Athlete"), value:4 },
				{ text:qsTr("Other"), value:5 } ]

		Component.onCompleted: {
			currentIndex = indexOfValue(userModel.role);
			bRoleOK = !userModel.isEmpty();
		}

		onActivated: (index) => {
			userModel.role = textAt(index);
			bRoleOK = true;
		}

		anchors {
			top: parent.top
			topMargin: -5
			left: lblRole.right
			leftMargin: 0
		}
	}

	Label {
		id: lblGoal
		text: userModel.columnLabel(8)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		width: parent.width*0.2

		anchors {
			top: lblRole.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
		}
	}

	TPComboBox {
		id: cboGoal
		height: controlsHeight
		enabled: bRoleOK
		model: goalModel
		width: parent.width*0.80

		readonly property var goalModel: [ { text:qsTr("General Fitness"), value:0 }, { text:qsTr("Loose Weight"), value:1 },
				{ text:qsTr("Improve Health"), value:2 }, { text:qsTr("Support for Other Sport"), value:3 },
				{ text:qsTr("Muscle Gain"), value:4 }, { text:qsTr("Strength"), value:5 }, { text:qsTr("Bodybuilding"), value:6 },
				{ text:qsTr("Other"), value:7 }]

		Component.onCompleted: {
			currentIndex = indexOfValue(userModel.goal);
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
		height: parent.height - 2*controlsHeight
		width: height
		radius: height/2
		layer.enabled: true
		visible: false
		color: "transparent"
		border.width: 1
		border.color: AppSettings.fontColor

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
