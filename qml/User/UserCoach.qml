pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Pages

ColumnLayout {
	id: userCoachModule

//public:
	required property TPPage parentPage
	property bool bReady: optPersonalUse.checked || optCoachUse.checked

//private:
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
		onChecked: (check) => AppUserModel.setIsClient(userCoachModule.userIdx, check);
	}

	TPRadioButtonOrCheckBox {
		id: optCoachUse
		text: qsTr("I will use this application to coach or train other people")
		multiLine: true
		Layout.fillWidth: true
		onChecked: (check) => AppUserModel.setIsCoach(userCoachModule.userIdx, check);
	}

	TPRadioButtonOrCheckBox {
		id: chkOnlineCoach
		text: qsTr("Make myself available online for TP users to contact me")
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		isChecked: AppUserModel.isCoachRegistered()
		multiLine: true
		enabled: AppUserModel.mainUserConfigured && AppUserModel.onlineAccount && optCoachUse.checked && userCoachModule.userIdx === 0
		Layout.fillWidth: true
		Layout.leftMargin: 10
		Layout.rightMargin: 10

		Connections {
			target: AppUserModel
			function onCoachOnlineStatus(registered: bool): void { chkOnlineCoach.isChecked = registered; }
		}

		onChecked: (check) => AppUserModel.setCoachPublicStatus(check);
	}

	TPFileViewer {
		fileOps: FileOperations {
			canAddFile: true
			useControls: true
		}
		useBackground: true
		missingFileInfo: qsTr("Choose your résumé file")
		Layout.preferredWidth: minimumWidth
		Layout.preferredHeight: minimumHeight
		Layout.alignment: Qt.AlignCenter
	}

	function getUserInfo(): void {
		optPersonalUse.isChecked = AppUserModel.isClient(userCoachModule.userIdx);
		optCoachUse.isChecked = AppUserModel.isCoach(userCoachModule.userIdx);
	}

	function focusOnFirstField(): void {
		optPersonalUse.forceActiveFocus();
	}
}
