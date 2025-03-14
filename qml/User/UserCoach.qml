import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Pages"

Frame {
	id: topFrame
	spacing: 0
	padding: 0
	height: moduleHeight
	implicitHeight: Math.min(height, moduleHeight)

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property int userRow
	required property TPPage parentPage
	property bool bReady: false
	property bool bCoachOK: false
	property bool bChooseResume: false
	property bool bResumeSent: false
	readonly property int moduleHeight: 0.35*appSettings.pageHeight
	readonly property int itemHeight: implicitHeight/4

	onBCoachOKChanged: bReady = bCoachOK;

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === userRow && field === 100)
				getUserInfo();
		}
	}

	onUserRowChanged: getUserInfo();
	Component.onCompleted: getUserInfo();

	TPRadioButton {
		id: optPersonalUse
		text: qsTr("I will use this application to track my own workouts only")
		multiLine: true
		actionable: userRow === 0
		height: itemHeight

		onClicked: {
			bReady = checked;
			if (checked)
				userModel.setAppUseMode(userRow, 1 + (chkHaveCoach.checked ? 2 : 0));
			optCoachUse.checked = false;
		}

		anchors {
			top: parent.top
			topMargin: -10
			left: parent.left
			right: parent.right
		}
	}

	TPRadioButton {
		id: optCoachUse
		text: qsTr("I will use this application to track my own workouts and/or coach or train other people")
		multiLine: true
		actionable: userRow === 0
		height: itemHeight

		onClicked: {
			bCoachOK = checked;
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (chkHaveCoach.checked ? 2 : 0));
			optPersonalUse.checked = false;
		}

		anchors {
			top: optPersonalUse.bottom
			topMargin: -5
			left: parent.left
			right: parent.right
		}
	}

	RowLayout {
		id: onlineCoachRow
		visible: appSettings.mainUserConfigured && optCoachUse.checked && userRow === 0
		spacing: 0
		height: itemHeight

		anchors {
			top: optCoachUse.bottom
			topMargin: 10
			left: parent.left
			right: parent.right
		}

		TPCheckBox {
			id: chkOnlineCoach
			text: qsTr("Make myself available online for TP users to contact me")
			checked: userModel.isCoachRegistered();
			multiLine: true
			actionable: userRow === 0
			height: itemHeight
			Layout.preferredWidth: parent.width/2

			Connections {
				target: userModel
				function onCoachOnlineStatus(registered: bool): void { chkOnlineCoach.checked = registered; }
			}

			onClicked: {
				userModel.setCoachPublicStatus(checked);
				if (checked)
					bCoachOK = bResumeSent;
			}
		}

		TPButton {
			id: btnSendResume
			text: qsTr("Send Résumé")
			autoResize: true
			enabled: chkOnlineCoach.checked
			Layout.preferredWidth: parent.width/2

			onClicked: bChooseResume = true;
		}
	}

	Loader {
		id: chooseResumeLoader
		active: bChooseResume
		asynchronous: true

		sourceComponent: FileDialog {
			id: chooseFileDlg
			title: qsTr("Choose the file to import from")
			defaultSuffix: "txt"
			nameFilters: [qsTr("Supported file types") + " (*.pdf *.odt *.docx)"]
			currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
			fileMode: FileDialog.OpenFile

			onAccepted: {
				userModel.uploadResume(currentFile);
				bChooseResume = false;
				bResumeSent = true;
				bCoachOK = true;
			}
			onRejected: bChooseResume = false;
		}

		onLoaded: item.open();
	}

	TPCheckBox {
		id: chkHaveCoach
		text: qsTr("I have a coach or a personal trainer")
		multiLine: true
		actionable: userRow === 0
		height: 25

		onClicked: {
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0)));
			else
				userModel.setAppUseMode(userRow, optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0));
		}

		anchors {
			bottom: parent.bottom
			bottomMargin: -10
			left: parent.left
			right: parent.right
		}
	}

	function getUserInfo(): void {
		if (userRow === -1)
			return;
		const app_use_mode = userModel.appUseMode(userRow);
		bReady = app_use_mode === 1 || app_use_mode === 3;
		optPersonalUse.checked = bReady;
		if (!bReady) {
			bCoachOK = app_use_mode === 2 || app_use_mode === 4;
			optCoachUse.checked = bCoachOK;
			chkHaveCoach.checked = app_use_mode === 3 || app_use_mode === 4;
		}
	}

	function focusOnFirstField(): void {
		optPersonalUse.forceActiveFocus();
	}
}
