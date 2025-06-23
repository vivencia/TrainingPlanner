import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Pages"

ColumnLayout {
	id: userModule
	spacing: 5

	required property int userRow
	required property TPPage parentPage
	property bool bReady: false
	property bool bCoachOK: false
	property bool bChooseResume: false
	property bool bRescindCoaching: false
	property bool bResumeSent: false

	onBCoachOKChanged: bReady = bCoachOK;

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === userRow && field >= 100)
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
		Layout.maximumWidth: userModule.width
		Layout.minimumWidth: userModule.width

		onClicked: {
			bReady = checked;
			if (checked)
				userModel.setAppUseMode(userRow, 1 + (chkHaveCoach.checked ? 2 : 0));
			optCoachUse.checked = false;
		}
	}

	TPRadioButton {
		id: optCoachUse
		text: qsTr("I will use this application to track my own workouts and/or coach or train other people")
		multiLine: true
		actionable: userRow === 0
		Layout.maximumWidth: userModule.width
		Layout.minimumWidth: userModule.width

		onClicked: {
			bCoachOK = checked;
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (chkHaveCoach.checked ? 2 : 0));
			optPersonalUse.checked = false;
		}
	}

	RowLayout {
		id: onlineCoachRow
		visible: userModel.mainUserConfigured && optCoachUse.checked && userRow === 0
		Layout.maximumWidth: userModule.width
		Layout.minimumWidth: userModule.width

		TPCheckBox {
			id: chkOnlineCoach
			text: qsTr("Make myself available online for TP users to contact me")
			checked: userModel.isCoachRegistered();
			multiLine: true
			actionable: userRow === 0
			Layout.preferredWidth: parent.width/2

			Connections {
				target: userModel
				function onCoachOnlineStatus(registered: bool): void { chkOnlineCoach.checked = registered; }
			}

			onClicked: {
				if (!checked && userModel.isCoachRegistered()) {
					if (userModel.haveClients)
						bRescindCoaching = true;
				}
				else {
					userModel.setCoachPublicStatus(checked);
					if (checked)
						bCoachOK = bResumeSent;
				}
			}
		}

		TPButton {
			id: btnSendResume
			text: qsTr("Send Résumé")
			autoSize: true
			enabled: chkOnlineCoach.checked
			Layout.preferredWidth: parent.width/2

			onClicked: bChooseResume = true;
		}
	}

	Loader {
		id: rescindCoachingLoader
		active: bRescindCoaching
		asynchronous: true

		sourceComponent: TPBalloonTip {
			id: rescindDlg
			title: qsTr("Rescind from coaching?")
			message: qsTr("You currently have clients that will no longer be your clients if you continue")
			imageSource: "warning"

			onButton1Clicked: {
				userModel.setCoachPublicStatus(false);
				bRescindCoaching = false;
			}
			onButton2Clicked: {
				chkOnlineCoach.checked = true;
				bRescindCoaching = false;
			}
		}

		onLoaded: item.show(-1);
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
		Layout.maximumWidth: userModule.width
		Layout.minimumWidth: userModule.width

		onClicked: {
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0)));
			else
				userModel.setAppUseMode(userRow, optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0));
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
