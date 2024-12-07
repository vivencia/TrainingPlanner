import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

Frame {
	id: topFrame
	padding: 0
	spacing: controlsSpacing
	height: minimumHeight
	implicitHeight: height
	implicitWidth: width

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property int userRow
	property bool bReady: true
	property bool bCoachOK: false
	readonly property int controlsSpacing: 10
	readonly property int minimumHeight: optPersonalUse.implicitHeight + optCoachUse.implicitHeight + chkHaveCoach.implicitHeight

	TPRadioButton {
		id: optPersonalUse
		text: qsTr("I will use this application to track my own workouts only")
		checked: userModel.appUseMode(userRow) === 1 || userModel.appUseMode(userRow) === 3;
		multiLine: true

		onClicked: {
			if (checked)
				userModel.setAppUseMode(userRow, 1 + (chkHaveCoach.checked ? 2 : 0));
			optCoachUse.checked = false;
		}

		anchors {
			top: parent.top
			topMargin: (topFrame.availableHeight - minimumHeight)/2
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
		multiLine: true

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
		multiLine: true

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

	function focusOnFirstField(): void {
		optPersonalUse.forceActiveFocus();
	}
}
