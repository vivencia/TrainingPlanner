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
	id: userCoachModule

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
	Component.onCompleted: {
		Layout.spacing = (Qt.platform.os !== "android") ? 30 : 10
		getUserInfo();
	}

	TPRadioButtonOrCheckBox {
		id: optPersonalUse
		text: qsTr("I will use this application to track my own workouts only")
		multiLine: true
		actionable: userRow === 0
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 30 : 10

		onClicked: {
			bReady = checked;
			if (checked)
				userModel.setAppUseMode(userRow, 1 + (chkHaveCoach.checked ? 2 : 0));
			optCoachUse.checked = false;
		}
	}

	TPRadioButtonOrCheckBox {
		id: optCoachUse
		text: qsTr("I will use this application to track my own workouts and/or coach or train other people")
		multiLine: true
		actionable: userRow === 0
		Layout.fillWidth: true

		onClicked: {
			bCoachOK = checked;
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (chkHaveCoach.checked ? 2 : 0));
			optPersonalUse.checked = false;
		}
	}

	TPRadioButtonOrCheckBox {
		id: chkOnlineCoach
		text: qsTr("Make myself available online for TP users to contact me")
		radio: false
		checked: userModel.isCoachRegistered();
		multiLine: true
		actionable: userRow === 0
		enabled: userModel.mainUserConfigured && userModel.onlineUser && optCoachUse.checked && userRow === 0
		Layout.fillWidth: true
		Layout.leftMargin: 10
		Layout.rightMargin: 10

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
		enabled: chkOnlineCoach.checked
		Layout.preferredWidth: parent.width/2
		Layout.alignment: Qt.AlignCenter

		onClicked: bChooseResume = true;
	}

	Loader {
		id: rescindCoachingLoader
		active: bRescindCoaching
		asynchronous: true

		sourceComponent: TPBalloonTip {
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
			nameFilters: [qsTr("Any file type") + " (*.*)"]
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

	TPRadioButtonOrCheckBox {
		id: chkHaveCoach
		text: qsTr("I intend to or do use a coach or a personal trainer")
		radio: false
		multiLine: true
		actionable: userRow === 0
		Layout.fillWidth: true

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
