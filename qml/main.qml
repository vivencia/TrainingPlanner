import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "Dialogs"
import "Pages"
import "TPWidgets"
import "User"

ApplicationWindow {
	id: mainwindow
	objectName: "mainWindow"
	width: appSettings.windowWidth
	height: appSettings.windowHeight
	visible: true
	title: "Training Planner"
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint | Qt.WA_KeepScreenOn :
				Qt.Window | Qt.CustomizeWindowHint & ~Qt.WindowMaximizeButtonHint

	property PagesListModel appPagesModel

	signal saveFileChosen(filepath: string);
	signal saveFileRejected(filepath: string);
	signal openFileChosen(filepath: string);
	signal openFileRejected(filepath: string);
	signal passwordDialogClosed(resultCode: int, password: string);
	signal removeNoLongerAvailableUser(row: int, remove: bool);
	signal unregisterUser(unregister: bool);
	signal revokeCoachStatus(new_use_opt: int, revoke: bool);
	signal revokeClientStatus(new_use_opt: int, revoke: bool);

	header: Loader {
		id: navBar
		active: userModel.mainUserConfigured
		asynchronous: true
		sourceComponent: NavBar {
			pagesModel: appPagesModel
		}
	}

	Loader {
		id: mainMenu
		active: userModel.mainUserConfigured
		asynchronous: true
		sourceComponent: MainMenu {
			rootPage: homePage
			pagesModel: appPagesModel
		}
	}

	Loader {
		id: appMessagesWidget
		active: userModel.mainUserConfigured && userModel.onlineAccount
		asynchronous: true
		sourceComponent: OnlineMessages{
			parentPage: homePage
		}
		onLoaded: item.open();
	}

	function openMainMenu(): void {
		mainMenu.item.open();
	}

	HomePage {
		id: homePage
		objectName: "homePage"
	}

	StackView {
		id: stackView
		objectName: "appStackView"
		initialItem: homePage
		anchors.fill: parent
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
	function pushOntoStack(page: Item, emit_signals: bool): void {
		if (stackView.currentItem === page)
			return;
		if (emit_signals)
			pageDeActivated_main(stackView.currentItem);
		if (stackView.find((item, index) => { return item === page; }))
			stackView.popToItem(page);
		else
			stackView.push(page);
		if (emit_signals)
			pageActivated_main(page);
	}

	function clearWindowsStack(): void {
		stackView.clear();
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

	Loader {
		id: importLoader
		asynchronous: true
		active: false

		sourceComponent: TPFileDialog {
			onDialogClosed: (result) => {
				if (result === 0)
					openFileChosen(appUtils.getCorrectPath(currentFile));
				else
					openFileRejected("");
				importLoader.active = false;
			}
		}

		onLoaded: item.show();
	}
	function chooseFileToImport(): void {
		importLoader.active = true;
	}

	property ImportDialog importConfirmDialog: null
	function createImportConfirmDialog(importOptions: list<string>, selectedFields: list<bool>): void {
		if (importConfirmDialog === null) {
			let component = Qt.createComponent("qrc:/qml/Dialogs/ImportDialog.qml", Qt.Asynchronous);

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

	Loader {
		id: saveDialogLoader
		asynchronous: true
		active: false

		property string suggestedFileName;
		sourceComponent: TPFileDialog {
			saveDialog: true
			chooseDialog: false
			onDialogClosed: (result) => {
				if (result === 0)
					saveFileChosen(appUtils.getCorrectPath(currentFile));
				else
					saveFileRejected("");
				saveDialogLoader.active = false;
			}
		}

		onLoaded: {
			item.suggestedName = suggestedFileName;
			item.show();
		}
	}
	function chooseFolderToSave(filename: string): void {
		saveDialogLoader.suggestedFileName = filename;
		saveDialogLoader.active = true;
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
					exitPopUp = component.createObject(contentItem, { parentPage: homePage, title: qsTr("Exit app?"), keepAbove: true });
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

	property TPBalloonTip unregisterUserDlg: null
	function showUnregisterUserDialog(title: string, message: string): void {
		if (unregisterUserDlg === null) {
			function createDialog() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					unregisterUserDlg = component.createObject(contentItem, { parentPage: homePage, title: title, message: message });
					unregisterUserDlg.button1Clicked.connect(function () { unregisterUser(true); });
					unregisterUserDlg.button2Clicked.connect(function () { unregisterUser(false); });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createDialog();
		}
		unregisterUserDlg.show(-1);
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

	property ChatWindow chatDlg: null
	function showChatWindow(chat_manager: ChatModel): void {
		if (chatDlg === null) {
			function createChatWindow() {
				let component = Qt.createComponent("qrc:/qml/User/ChatWindow.qml", Qt.Asynchronous);

				function finishCreation() {
					chatDlg = component.createObject(contentItem, { parentPage: homePage, chatManager: chat_manager });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createChatWindow();
		}
		chatDlg.show1(-1);
	}
} //ApplicationWindow
