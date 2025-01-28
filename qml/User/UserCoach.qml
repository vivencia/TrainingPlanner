import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

Frame {
	id: topFrame
	height: moduleHeight
	implicitHeight: Math.min(height, moduleHeight)
	spacing: 5
	padding: 0

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property int userRow
	property bool bReady: false
	property bool bCoachOK: false
	property bool bChooseResume: false
	property bool bResumeSent: false
	readonly property int moduleHeight: 0.25*appSettings.pageHeight
	readonly property int itemHeight: implicitHeight/3

	onBCoachOKChanged: bReady = bCoachOK;

	TPRadioButton {
		id: optPersonalUse
		text: qsTr("I will use this application to track my own workouts only")
		checked: userModel.appUseMode(userRow) === 1 || userModel.appUseMode(userRow) === 3;
		multiLine: true
		height: itemHeight

		onClicked: {
			bReady = checked;
			if (checked)
				userModel.setAppUseMode(userRow, 1 + (chkHaveCoach.checked ? 2 : 0));
			optCoachUse.checked = false;
		}

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}
	}

	TPRadioButton {
		id: optCoachUse
		text: qsTr("I will use this application to track my own workouts and/or coach or train other people")
		checked: userModel.appUseMode(userRow) === 2 || userModel.appUseMode(userRow) === 4;
		multiLine: true
		height: itemHeight

		onClicked: {
			bCoachOK = checked;
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (chkHaveCoach.checked ? 2 : 0));
			optPersonalUse.checked = false;
		}

		anchors {
			top: optPersonalUse.bottom
			left: parent.left
			right: parent.right
		}
	}

	RowLayout {
		id: onlineCoachRow
		visible: optCoachUse.checked && userRow === 0
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
			multiLine: true
			height: itemHeight
			Layout.preferredWidth: parent.width/2

			Component.onCompleted: userModel.isCoachAlreadyRegisteredOnline(userRow);

			Connections {
				target: userModel
				function onCoachOnlineStatus(registered: bool): void { chkOnlineCoach.checked = registered; }
			}

			onClicked: {
				userModel.setCoachPublicStatus(userRow, checked);
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
			nameFilters: ["PDFs (*.pdf)", "ODFs (*.odf)", "DOCs (*.docx)"]
			currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
			fileMode: FileDialog.OpenFile

			onAccepted: {
				userModel.uploadResume(userRow, currentFile);
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
		checked: userModel.appUseMode(userRow) === 3 || userModel.appUseMode(userRow) === 4;
		multiLine: true
		height: itemHeight

		onClicked: {
			if (checked)
				userModel.setAppUseMode(userRow, 2 + (optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0)));
			else
				userModel.setAppUseMode(userRow, optPersonalUse.checked ? 1 : (optCoachUse.checked ? 2 : 0));
		}

		anchors {
			top: onlineCoachRow.visible ? onlineCoachRow.bottom : optCoachUse.bottom
			topMargin: 20
			left: parent.left
			right: parent.right
		}
	}

	TPButton {
		id: btnFindCoachOnline
		text: qsTr("Look online for available coaches");
		visible: chkHaveCoach.checked

		onClicked: displayOnlineCoachesMenu();

		anchors {
			top: chkHaveCoach.bottom
			topMargin: 5
			left: parent.left
			right: parent.right
		}
	}

	function focusOnFirstField(): void {
		optPersonalUse.forceActiveFocus();
	}
}
