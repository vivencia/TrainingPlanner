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

	property bool bReady: bCoachOK
	property bool bCoachOK: false
	readonly property int controlsSpacing: 10

	TPRadioButton {
		id: optPersonalUse
		text: qsTr("I will use this application to track my own workouts only")

		onClicked: {
			if (checked)
				userModel.appUseMode = 1 + chkHaveCoach.checked ? 2 : 0;
			bCoachOK = userModel.appUseMode !== 2;
			optCoachUse.checked = false;
		}

		Component.onCompleted: {
			checked = userModel.appUseMode === 1 || userModel.appUseMode === 3;
			bCoachOK = true;
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

		onClicked: {
			if (checked)
				userModel.appUseMode = 2 + chkHaveCoach.checked ? 2 : 0;
			bCoachOK = userModel.appUseMode !== 2;
			optPersonalUse.checked = false;
		}

		Component.onCompleted: {
			checked = userModel.appUseMode === 2 || userModel.appUseMode === 4;
			bCoachOK = true;
		}

		anchors {
			top: optPersonalUse.bottom
			topMargin: 3*controlsSpacing
			left: parent.left
			leftMargin: 10
			right: parent.right
			rightMargin: 10
		}
	}

	TPCheckBox {
		id: chkHaveCoach
		text: qsTr("I have a coach or a personal trainer")
		checked: false

		onClicked: {
			if (checked)
				userModel.appUseMode = 2 + (optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0))
			bCoachOK = userModel.appUseMode !== 2;
		}

		Component.onCompleted: {
			checked = userModel.appUseMode === 3 || userModel.appUseMode === 4;
			bCoachOK = true;
		}

		anchors {
			top: optCoachUse.bottom
			topMargin: 3*controlsSpacing
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
