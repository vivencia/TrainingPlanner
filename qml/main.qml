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
	width: 300
	height: 640
	visible: true
	title: "Training Planner"
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint | Qt.WA_KeepScreenOn : Qt.Window

	property bool bBackButtonEnabled: appSettings.mainUserConfigured
	property int backKey

	signal saveFileChosen(filepath: string);
	signal saveFileRejected(filepath: string);
	signal openFileChosen(filepath: string);
	signal openFileRejected(filepath: string);

	Component.onCompleted: {
		if (Qt.platform.os === "android")
			backKey = Qt.Key_Back;
		else
			backKey = Qt.Key_Left;

		contentItem.Keys.pressed.connect( function(event) {
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

		background: Rectangle {
			color: appSettings.primaryDarkColor
			opacity: 0.7
		}
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
	function showFirstUseTimeDialog() {
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
	function popFromStack(page: Item) {
		pageDeActivated_main(stackView.currentItem);
		if (page)
			stackView.pop(page);
		else
			stackView.pop();
		pageActivated_main(stackView.currentItem);
	}

	signal pageActivated_main(Item page);
	function pushOntoStack(page: Item) {
		if (stackView.currentItem === page)
			return;
		pageDeActivated_main(stackView.currentItem);
		stackView.push(page);
		pageActivated_main(page);
	}

	function confirmImport(message: string) {
		importConfirmDialog.title = qsTr("Proceed with action?");
		importConfirmDialog.message = message;
		importConfirmDialog.show(-1);
	}

	property TPImportDialog importOpenDialog: null
	function chooseFileToImport() {
		if (importOpenDialog === null) {
			function createImportDialog() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPImportDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					importOpenDialog = component.createObject(contentItem, {});
					importOpenDialog.open();
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createImportDialog();
		}
		else
			importOpenDialog.open();
	}

	property ImportDialog importConfirmDialog: null
	function createImportConfirmDialog(itemManager: QmlItemManager, importOptions: var, selectedFields: var) {
		if (importConfirmDialog === null) {
			var component = Qt.createComponent("qrc:/qml/Dialogs/ImportDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				importConfirmDialog = component.createObject(contentItem, {parentPage: stackView.currentItem,
							itemManager: itemManager, importOptions: importOptions, selectedFields: selectedFields});
				importConfirmDialog.show(-1);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		else
		{
			importConfirmDialog.itemManager = itemManager;
			importConfirmDialog.parentPage = stackView.currentItem;
			importConfirmDialog.importOptions = importOptions;
			importConfirmDialog.selectedFields = selectedFields;
			importConfirmDialog.show(-1);
		}
	}

	property TPSaveDialog saveDialog: null
	function chooseFolderToSave(filename: string) {
		if (saveDialog === null) {
			function createSaveDialog() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPSaveDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					saveDialog = component.createObject(contentItem, {});
					saveDialog.init(filename);
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createSaveDialog();
		}
		else
			saveDialog.init(filename);
	}

	TPBalloonTip {
		id: textCopiedInfo
		height: 40
		message: qsTr("Text copied to the clipboard")
		parentPage: homePage
	}
	function showTextCopiedMessage() {
		textCopiedInfo.showTimed(3000, 0);
	}

	TPBalloonTip {
		id: activityFinishedTip
		imageSource: "import.png"
		button1Text: "OK"
		parentPage: homePage
	}

	function createShortCut(label: string, page: Item, clickid: int) {
		mainMenu.createShortCut(label, page, clickid);
	}

	function displayResultMessage(title: string, message: string) {
		activityFinishedTip.title = title;
		activityFinishedTip.message = message;
		activityFinishedTip.showTimed(5000, 0);
	}
} //ApplicationWindow
