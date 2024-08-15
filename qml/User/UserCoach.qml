import QtQuick
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

import ".."
import "../TPWidgets"

Frame {
	implicitHeight: height
	implicitWidth: width
	padding: 0
	spacing: controlsSpacing

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property int userRow
	property bool bReady: true
	property bool bCoachOK: false
	readonly property int controlsSpacing: 10

	TPRadioButton {
		id: optPersonalUse
		text: qsTr("I will use this application to track my own workouts only")
		checked: userModel.appUseMode(userRow) === 1 || userModel.appUseMode(userRow) === 3;

		onClicked: {
			if (checked)
				userModel.setAppUseMode(userRow, 1 + (chkHaveCoach.checked ? 2 : 0));
			optCoachUse.checked = false;
		}

		anchors {
			top: parent.top
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 10
			right: parent.right
			rightMargin: 10
		}
	}

	TPRadioButton {
		id: optCoachUse
		text: qsTr("I will use this application to track my own workouts and/or coach or train other people")
		checked: userModel.appUseMode(userRow) === 2 || userModel.appUseMode(userRow) === 4;

		onClicked: {
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (chkHaveCoach.checked ? 2 : 0));
			optPersonalUse.checked = false;
		}

		anchors {
			top: optPersonalUse.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 10
			right: parent.right
			rightMargin: 10
		}
	}

	TPCheckBox {
		id: chkHaveCoach
		text: qsTr("I have a coach or a personal trainer")
		checked: userModel.appUseMode(userRow) === 3 || userModel.appUseMode(userRow) === 4;

		onClicked: {
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0)));
			else
				userModel.setAppUseMode(userRow, optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0));
		}

		anchors {
			top: optCoachUse.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 10
			right: parent.right
			rightMargin: 10
		}
	}

	Label {
		text: qsTr("Data about them can be configured at a later point")
		visible: parent.objectName === "firstTimerDlg"
		enabled: chkHaveCoach.checked

		anchors {
			top: chkHaveCoach.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 10
			right: parent.right
			rightMargin: 10
		}
	}

	function focusOnFirstField() {
		optPersonalUse.forceActiveFocus();
	}
}
