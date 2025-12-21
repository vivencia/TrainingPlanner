import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "ExercisesAndSets"
import "Dialogs"
import "Pages"
import "TPWidgets"
import "User"

ApplicationWindow {
	id: mainwindow
	visible: true
	title: "TraininPlanner Tests"
	objectName: "mainWindow"
	width: appSettings.windowWidth
	height: appSettings.windowHeight
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint | Qt.WA_KeepScreenOn :
				Qt.Window | Qt.CustomizeWindowHint & ~Qt.WindowMaximizeButtonHint

	signal pageActivated_main(Item page);
	signal pageDeActivated_main(Item page);
	signal passwordDialogClosed(resultCode: int, password: string);
	signal saveFileChosen(filepath: string);
	signal saveFileRejected(filepath: string);
	signal openFileChosen(filepath: string, content_type: int);
	signal openFileRejected(filepath: string);

	property PagesListModel appPagesModel

	//Component.onCompleted: timePicker.show1();

	TPPage {
		id: homePage
		anchors.fill: parent

		Rectangle {
			x: (parent.width - width) / 2;
			y: (parent.height - height) / 2;
			width: parent.width * 0.8
			height: 300
			color: appSettings.paneBackgroundColor
			border.width: 2
			border.color: appSettings.fontColor

			TPMultiLineEdit {
				anchors.fill: parent
				anchors.margins: 20
			}
		}
	}

	TPBalloonTip {
		id: textCopiedInfo
		height: 40
		message: qsTr("Text copied to the clipboard")
		button1Text: ""
		button2Text: ""
		parentPage: homePage
	}

	function showTextCopiedMessage(): void {
		textCopiedInfo.showTimed(3000, 0);
	}

	TPBalloonTip {
		id: generalMessagesPopup
		parentPage: homePage
		button1Text: ""
		button2Text: ""
	}

	function displayResultMessage(title: string, message: string, img_src: string, msecs: int): void {
		generalMessagesPopup.title = title;
		generalMessagesPopup.message = message;
		generalMessagesPopup.imageSource = img_src;
		if (msecs > 0)
			generalMessagesPopup.showTimed(msecs, 0);
		else
			generalMessagesPopup.show(0);
	}

	property PasswordDialog passwdDlg: null
	function showPasswordDialog(title: string, message: string): void {
		if (passwdDlg === null) {
			function createPasswordDialog() {
				let component = Qt.createComponent("qrc:/qml/Dialogs/PasswordDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					passwdDlg = component.createObject(contentItem, { parentPage: homePage, title: title, message: message });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createPasswordDialog();
		}
		passwdDlg.show(-1);
	}
}
