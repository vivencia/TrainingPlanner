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

	property bool bBackButtonEnabled: appSettings.mainUserConfigured
	property int backKey

	signal saveFileChosen(filepath: string);
	signal saveFileRejected(filepath: string);
	signal openFileChosen(filepath: string, filetype: int);
	signal openFileRejected(filepath: string);

	Component.onCompleted: {
		if (Qt.platform.os === "android")
			backKey = Qt.Key_Back;
		else
			backKey = Qt.Key_Left;

		contentItem.Keys.pressed.connect(function(event) {
			if (event.key === backKey) {
				event.accepted = true;
				if (stackView.depth >= 2)
					popFromStack();
				else
					close();
			}
		});
	}

	header: NavBar {
		id: navBar
	}

	MainMenu {
		id: mainMenu
		objectName: "mainMenu"
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

	/*FirstTimeDialog {
		DBUserModel {
			id: userModel
		}
		id: firstTimeDlgg
		parentPage: homePage
	}*/

	property var firstTimeDlg: null
	function showFirstUseTimeDialog(): void {
		function createFirstTimeDialog() {
			var component = Qt.createComponent("qrc:/qml/Dialogs/FirstTimeDialog.qml", Qt.Asynchronous);

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
		//firstTimeDlgg.open();
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

	function confirmImport(message: string): void {
		importConfirmDialog.title = qsTr("Proceed with action?");
		importConfirmDialog.message = message;
		importConfirmDialog.show(-1);
	}

	property TPImportDialog importOpenDialog: null
	function chooseFileToImport(filetype: int): void {
		if (importOpenDialog === null) {
			function createImportDialog() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPImportDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					importOpenDialog = component.createObject(contentItem, {fileType: filetype});
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
				importConfirmDialog = component.createObject(contentItem, {parentPage: stackView.currentItem, importOptions: importOptions,
							selectedFields: selectedFields});
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

	property SelectMesoForImport selectMesoDlg: null
	function selectMesoDialog(msg: string, mesoInfo: list<string>, idxsList: list<int>): void {
		if (selectMesoDlg === null) {
			var component = Qt.createComponent("qrc:/qml/Dialogs/SelectMesoForImport.qml", Qt.Asynchronous);

			function finishCreation() {
				selectMesoDlg = component.createObject(contentItem, {parentPage: stackView.currentItem});
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		selectMesoDlg.parentPage = stackView.currentItem;
		selectMesoDlg.message = msg;
		selectMesoDlg.mesosList = mesoInfo;
		selectMesoDlg.idxsList = idxsList;
		selectMesoDlg.show(-1);
	}

	property TPSaveDialog saveDialog: null
	function chooseFolderToSave(filename: string): void {
		if (saveDialog === null) {
			function createSaveDialog() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPSaveDialog.qml", Qt.Asynchronous);

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

	TPBalloonTip {
		id: textCopiedInfo
		height: 40
		message: qsTr("Text copied to the clipboard")
		parentPage: homePage
	}
	function showTextCopiedMessage(): void {
		textCopiedInfo.showTimed(3000, 0);
	}

	TPBalloonTip {
		id: activityFinishedTip
		imageSource: "import.png"
		button1Text: "OK"
		parentPage: homePage
	}

	function displayResultMessage(title: string, message: string): void {
		activityFinishedTip.title = title;
		activityFinishedTip.message = message;
		activityFinishedTip.showTimed(5000, 0);
	}
} //ApplicationWindow
