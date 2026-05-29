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
	required property TPPage parentPage
	property bool bReady: optPersonalUse.checked || optCoachUse.checked

//private:
	property bool bChooseResume: false
	property bool bRescindCoach: false
	property bool bResumeSent: false
	readonly property int userIdx: 0

	Component.onCompleted: getUserInfo();

	Connections {
		target: AppUserModel
		function onUserModified(row: int, field: int): void {
			if (row === userCoachModule.userIdx)
				userCoachModule.getUserInfo();
		}
	}

	TPRadioButtonOrCheckBox {
		id: optPersonalUse
		text: qsTr("I will use this application to track my workouts and training regimen")
		multiLine: true
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 30 : 10
		onClicked: AppUserModel.setIsClient(userCoachModule.userIdx, checked);
	}

	TPRadioButtonOrCheckBox {
		id: optCoachUse
		text: qsTr("I will use this application to coach or train other people")
		multiLine: true
		Layout.fillWidth: true
		onClicked: AppUserModel.setIsCoach(userCoachModule.userIdx, checked);
	}

	TPRadioButtonOrCheckBox {
		id: chkOnlineCoach
		text: qsTr("Make myself available online for TP users to contact me")
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		checked: AppUserModel.isCoachRegistered()
		multiLine: true
		enabled: AppUserModel.mainUserConfigured && AppUserModel.onlineAccount && optCoachUse.checked && userCoachModule.userIdx === 0
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
			else
				AppUserModel.setCoachPublicStatus(checked);
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
			fileType: AppUtils.FT_PDF|AppUtils.FT_OPEN_DOCUMENT|AppUtils.FT_MS_DOCUMENT

			onAccepted: {
				AppUserModel.uploadResume(currentFile);
				userCoachModule.bChooseResume = false;
				userCoachModule.bResumeSent = true;
			}
			onRejected: userCoachModule.bChooseResume = false;
			Component.onCompleted: chooseResumeLoader._file_dialog = this
		}

		onLoaded: _file_dialog.open();
	}

	function getUserInfo(): void {
		optPersonalUse.checked = AppUserModel.isClient(userCoachModule.userIdx);
		optCoachUse.checked = AppUserModel.isCoach(userCoachModule.userIdx);
	}

	function focusOnFirstField(): void {
		optPersonalUse.forceActiveFocus();
	}
}
