pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Pages
import TpQml.Dialogs

ColumnLayout {
	id: userCoachModule

//public:
	required property int userRow
	required property TPPage parentPage
	property bool bReady: false

//private:
	property bool bCoachOK: false
	property bool bChooseResume: false
	property bool bRescindCoach: false
	property bool bResumeSent: false

	onBCoachOKChanged: bReady = userCoachModule.bCoachOK;

	Connections {
		target: AppUserModel
		function onUserModified(row: int, field: int): void {
			if (row === userCoachModule.userRow && field >= 100)
				userCoachModule.getUserInfo();
		}
	}

	onUserRowChanged: getUserInfo();
	Component.onCompleted: {
		Layout.spacing = (Qt.platform.os !== "android") ? 30 : 10
		getUserInfo();
	}

	TPRadioButtonOrCheckBox {
		id: optPersonalUse
		text: qsTr("I will use this application to track my workouts and training regimen")
		multiLine: true
		actionable: userCoachModule.userRow === 0
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 30 : 10

		onClicked: {
			userCoachModule.bReady = checked;
			if (checked)
				AppUserModel.setAppUseMode(userCoachModule.userRow, optCoachUse.checked ?
											   AppUserModel.USEMODE_SINGLE_USER_WITH_COACH : AppUserModel.USEMODE_SINGLE_USER);
			optCoachUse.checked = false;
		}
	}

	TPRadioButtonOrCheckBox {
		id: optCoachUse
		text: qsTr("I will use this application to coach or train other people")
		multiLine: true
		actionable: userCoachModule.userRow === 0
		Layout.fillWidth: true

		onClicked: {
			userCoachModule.bCoachOK = checked;
			if (checked)
				AppUserModel.setAppUseMode(userCoachModule.userRow, optPersonalUse.checked ?
											   AppUserModel.USEMODE_COACH_USER_WITH_COACH : AppUserModel.USEMODE_SINGLE_COACH);
			optPersonalUse.checked = false;
		}
	}

	TPRadioButtonOrCheckBox {
		id: chkOnlineCoach
		text: qsTr("Make myself available online for TP users to contact me")
		radio: false
		checked: AppUserModel.isCoachRegistered();
		multiLine: true
		actionable: userCoachModule.userRow === 0
		enabled: AppUserModel.mainUserConfigured && AppUserModel.onlineAccount && optCoachUse.checked && userCoachModule.userRow === 0
		Layout.fillWidth: true
		Layout.leftMargin: 10
		Layout.rightMargin: 10

		Connections {
			target: AppUserModel
			function onCoachOnlineStatus(registered: bool): void { chkOnlineCoach.checked = registered; }
		}

		onClicked: {
			if (!checked && AppUserModel.isCoachRegistered()) {
				if (AppUserModel.currentClients.count > 0)
					userCoachModule.bRescindCoach = true;
			}
			else {
				AppUserModel.setCoachPublicStatus(checked);
				if (checked)
					userCoachModule.bCoachOK = userCoachModule.bResumeSent;
			}
		}
	}

	TPButton {
		id: btnSendResume
		text: qsTr("Send Résumé")
		rounded: false
		enabled: chkOnlineCoach.checked
		Layout.preferredWidth: parent.width/2
		Layout.alignment: Qt.AlignCenter

		onClicked: userCoachModule.bChooseResume = true;
	}

	Loader {
		id: rescindCoachLoader
		active: userCoachModule.bRescindCoach
		asynchronous: true

		property TPBalloonTip _rescind_dlg

		sourceComponent: TPBalloonTip {
			title: qsTr("Rescind from coaching?")
			message: qsTr("You currently have clients that will no longer be your clients if you continue")
			imageSource: "warning"

			onButton1Clicked: {
				AppUserModel.setCoachPublicStatus(false);
				userCoachModule.bRescindCoach = false;
			}
			onButton2Clicked: {
				chkOnlineCoach.checked = true;
				userCoachModule.bRescindCoach = false;
			}

			Component.onCompleted: rescindCoachLoader._rescind_dlg = this;
		}

		onLoaded: _rescind_dlg.tpOpen();
	}

	Loader {
		id: chooseResumeLoader
		active: userCoachModule.bChooseResume
		asynchronous: true

		property TPFileDialog _file_dialog

		sourceComponent: TPFileDialog {
			id: chooseFileDlg
			title: qsTr("Choose the file to import from")
			includePDFFilter: true
			includeDocFilesFilter: true

			onAccepted: {
				AppUserModel.uploadResume(currentFile);
				userCoachModule.bChooseResume = false;
				userCoachModule.bResumeSent = true;
				userCoachModule.bCoachOK = true;
			}
			onRejected: userCoachModule.bChooseResume = false;
			Component.onCompleted: chooseResumeLoader._file_dialog = this
		}

		onLoaded: _file_dialog.open();
	}

	function getUserInfo(): void {
		if (userCoachModule.userRow === -1)
			return;
		const app_use_mode = AppUserModel.appUseMode(userCoachModule.userRow);
		userCoachModule.bReady = app_use_mode === AppUserModel.USEMODE_SINGLE_USER || app_use_mode === AppUserModel.USEMODE_SINGLE_USER_WITH_COACH;
		optPersonalUse.checked = userCoachModule.bReady;
		if (!userCoachModule.bReady) {
			userCoachModule.bCoachOK = app_use_mode === AppUserModel.USEMODE_SINGLE_COACH || app_use_mode === AppUserModel.USEMODE_COACH_USER_WITH_COACH;
			optCoachUse.checked = userCoachModule.bCoachOK;
		}
	}

	function focusOnFirstField(): void {
		optPersonalUse.forceActiveFocus();
	}
}
