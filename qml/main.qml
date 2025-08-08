import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "Dialogs"
import "Pages"
import "TPWidgets"

ApplicationWindow {
	id: mainwindow
	objectName: "mainWindow"
	width: appSettings.windowWidth
	height: appSettings.windowHeight
	visible: true
	title: "Training Planner"
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint | Qt.WA_KeepScreenOn : Qt.Window

	property int n_dialogs_open: 0

	onN_dialogs_openChanged: {
		if (n_dialogs_open < 0)
			n_dialogs_open = 0;
	}

	signal saveFileChosen(filepath: string);
	signal saveFileRejected(filepath: string);
	signal openFileChosen(filepath: string, content_type: int);
	signal openFileRejected(filepath: string);
	signal passwordDialogClosed(resultCode: int, password: string);
	signal removeNoLongerAvailableUser(row: int, remove: bool);
	signal revokeCoachStatus(new_use_opt: int, revoke: bool);
	signal revokeClientStatus(new_use_opt: int, revoke: bool);
	signal closeDialog();

	header: Loader {
		id: navBar
		active: userModel.mainUserConfigured
		asynchronous: true
		source: "qrc:/qml/Dialogs/NavBar.qml"
	}

	Loader {
		id: mainMenu
		active: userModel.mainUserConfigured
		asynchronous: true
		source: "qrc:/qml/Dialogs/MainMenu.qml"
	}

	Loader {
		id: appMessagesWidget
		active: appMessages ? appMessages.count > 0 : false
		asynchronous: true
		source: "qrc:/qml/Dialogs/OnlineMessages.qml"
	}

	function openMainMenu(): void {
		mainMenu.item.open();
	}

	Flickable {
		width: parent.width
		height: parent.height
		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator {}

		HomePage {
			id: homePage
			objectName: "homePage"
		}

		StackView {
			id: stackView
			objectName: "appStackView"
			anchors.fill: parent
			initialItem: homePage
		}
	}

	signal pageDeActivated_main(Item page);
	function popFromStack(page: Item): void {
		pageDeActivated_main(stackView.currentItem);
		if (page) {
			if (stackView.currentItem !== page) {
				let items = [];
				for (let i = 0; i < stackView.depth; ++i) {
					if (stackView.get(i) !== page)
						items.push(stackView.get(i));
				}
				stackView.clear();
				for (let x = 0; x < items.length; ++x)
					stackView.push(items[x], {}, StackView.Immediate);
			}
			else
				stackView.pop();
		}
		else
			stackView.pop();
		pageActivated_main(stackView.currentItem);
	}

	signal pageActivated_main(Item page);
	function pushOntoStack(page: Item): void {
		if (stackView.currentItem === page)
			return;
		pageDeActivated_main(stackView.currentItem);
		if (stackView.find((item, index) => { return item === page; }))
			stackView.popToItem(page);
		else
			stackView.push(page);
		pageActivated_main(page);
	}

	function goHome(): void {
		pageDeActivated_main(stackView.currentItem);
		stackView.pop(stackView.get(0));
		pageActivated_main(stackView.currentItem);
	}

	function confirmImport(message: string): void {
		importConfirmDialog.title = qsTr("Proceed with action?");
		importConfirmDialog.message = message;
		importConfirmDialog.show(-1);
	}

	/*FirstTimeDialog {
		DBUserModel {
			id: userModel
		}
		id: firstTimeDlgg
		parentPage: homePage
	}*/

	property FirstTimeDialog firstTimeDlg: null
	function showFirstTimeUseDialog(): void {
		function createFirstTimeDialog() {
			let component = Qt.createComponent("qrc:/qml/Dialogs/FirstTimeDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				firstTimeDlg = component.createObject(homePage, { parentPage: homePage });
				firstTimeDlg.open();
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		createFirstTimeDialog();
		//firstTimeDlgg.show1(-1);
	}

	property TPImportDialog importOpenDialog: null
	function chooseFileToImport(content_type: int): void {
		if (importOpenDialog === null) {
			function createImportDialog() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPImportDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					importOpenDialog = component.createObject(contentItem, {contentType: content_type});
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createImportDialog();
		}
		importOpenDialog.open();
	}

	property ImportDialog importConfirmDialog: null
	function createImportConfirmDialog(importOptions: list<string>, selectedFields: list<bool>): void {
		if (importConfirmDialog === null) {
			var component = Qt.createComponent("qrc:/qml/Dialogs/ImportDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				importConfirmDialog = component.createObject(contentItem, {
								parentPage: stackView.currentItem, importOptions: importOptions, selectedFields: selectedFields});
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		else {
			importConfirmDialog.parentPage = stackView.currentItem;
			importConfirmDialog.selectedFields = selectedFields;
			importConfirmDialog.importOptions = 0;
			importConfirmDialog.importOptions = importOptions;
		}
		importConfirmDialog.show(-1);
	}

	property TPSaveDialog saveDialog: null
	function chooseFolderToSave(filename: string): void {
		if (saveDialog === null) {
			function createSaveDialog() {
				let component = Qt.createComponent("qrc:/qml/Dialogs/TPSaveDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					saveDialog = component.createObject(contentItem, {});
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createSaveDialog();
		}
		saveDialog.init(filename);
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

	property TPBalloonTip userNoLongerAvailableDlg: null
	function showUserNoLongerAvailable(row: int, title: string, message: string): void {
		if (userNoLongerAvailableDlg === null) {
			function createDialog() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					userNoLongerAvailableDlg = component.createObject(contentItem, { parentPage: homePage, title: title, message: message, keepAbove: true });
					userNoLongerAvailableDlg.button1Clicked.connect(function () { removeNoLongerAvailableUser(row, true); });
					userNoLongerAvailableDlg.button2Clicked.connect(function () { removeNoLongerAvailableUser(row, false); });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createDialog();
		}
		userNoLongerAvailableDlg.show(-1);
	}

	property TPBalloonTip exitPopUp: null
	function showExitPopUp(): void {
		if (exitPopUp === null) {
			function createDialog() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					exitPopUp = component.createObject(contentItem, { parentPage: homePage, title: qsTr("Sair do app?"), keepAbove: true });
					exitPopUp.button1Clicked.connect(function () { close(); });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createDialog();
		}
		exitPopUp.show(-2);
	}

	property TPBalloonTip revokeCoachStatusDlg: null
	function showRevokeCoachStatus(new_use_opt: int, title: string, message: string): void {
		if (revokeCoachStatusDlg === null) {
			function createDialog() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					revokeCoachStatusDlg = component.createObject(contentItem, { parentPage: homePage, title: title, message: message });
					revokeCoachStatusDlg.button1Clicked.connect(function () { revokeCoachStatus(new_use_opt, true); });
					revokeCoachStatusDlg.button2Clicked.connect(function () { revokeCoachStatus(new_use_opt, false); });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createDialog();
		}
		revokeCoachStatusDlg.show(-1);
	}

	property TPBalloonTip revokeClientStatusDlg: null
	function showRevokeClientStatus(new_use_opt: int, title: string, message: string): void {
		if (revokeClientStatusDlg === null) {
			function createDialog() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					revokeClientStatusDlg = component.createObject(contentItem, { parentPage: homePage, title:title, message:message });
					revokeClientStatusDlg.button1Clicked.connect(function () { revokeClientStatus(new_use_opt, true); });
					revokeClientStatusDlg.button2Clicked.connect(function () { revokeClientStatus(new_use_opt, false); });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createDialog();
		}
		revokeClientStatusDlg.show(-1);
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
		id: activityFinishedTip
		parentPage: homePage
		button1Text: ""
		button2Text: ""
	}

	function displayResultMessage(title: string, message: string, img_src: string, msecs: int): void {
		activityFinishedTip.title = title;
		activityFinishedTip.message = message;
		activityFinishedTip.imageSource = img_src;
		if (msecs > 0)
			activityFinishedTip.showTimed(msecs, 0);
		else
			activityFinishedTip.show(0);
	}
} //ApplicationWindow
