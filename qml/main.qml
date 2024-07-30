import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

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

	readonly property string lightIconFolder: "white/"
	readonly property string darkIconFolder: "black/"
	readonly property int windowWidth: width
	readonly property int windowHeight: contentItem.height

	property var importMessageDialog: null
	property var importOpenDialog: null
	property var saveDialog: null
	property string importExportFilename
	property bool bBackButtonEnabled: true

	property int backKey

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
			color: AppSettings.primaryDarkColor
			opacity: 0.7
		}
	}

	MainMenu {
		id: mainMenu
	}

	Flickable {
		width: parent.width
		height: parent.height
		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator {}

		HomePage {
			id: homePage
		}

		StackView {
			id: stackView
			objectName: "appStackView"
			anchors.fill: parent
			initialItem: homePage
		}
	}

	footer: TabBar {
		id: tabMain

		TabButton {
			text: qsTr("HOME")
			enabled: stackView.depth >= 2

			Image {
				source: "qrc:/images/"+darkIconFolder+"home.png"
				height: 30
				width: 30
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.left
				anchors.leftMargin: 10
			}

			onClicked: {
				stackView.pop(stackView.get(0));
				btnWorkoutEnabled();
			}
		}

		TabButton {
			id: btnWorkout
			text: "          " + qsTr("Today's Workout")
			font.pointSize: AppSettings.fontSizeText

			Image {
				source: "qrc:/images/"+darkIconFolder+"exercises.png"
				height: 40
				width: 40
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.left
				anchors.leftMargin: 10
			}

			onClicked: appDB.getTrainingDay(new Date());
		} //TabButton
	} //footer

	function init() {
		homePage.setViewModel();
		mesocyclesModel.currentRowChanged.connect(btnWorkoutEnabled);
		btnWorkoutEnabled();
		if (AppSettings.firstTime) {
			bBackButtonEnabled = false;
			stackView.push("qrc:/qml/Pages/SettingsPage.qml");
		}
		else
			checkInitialArguments();
	}

	function checkInitialArguments() {
		if (Qt.platform.os === "android")
			appDB.checkPendingIntents();
		else
			appDB.processArguments();
	}

	function btnWorkoutEnabled() {
		if (stackView.depth === 1)
			btnWorkout.enabled = mesocyclesModel.isDateWithinCurrentMeso(new Date());
		else
			btnWorkout.enabled = false;
	}

	function popFromStack(page: Item) {
		if (page)
			stackView.pop(page);
		else
			stackView.pop();
		btnWorkoutEnabled();
	}

	function pushOntoStack(page: Item) {
		stackView.push(page);
		btnWorkoutEnabled();
	}

	function createShortCut(label: string, object: Item, clickid: int) {
		mainMenu.createShortCut(label, object, clickid);
	}

	TPBalloonTip {
		id: importConfirmDialog
		imageSource: "import.png"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		parentPage: homePage

		onButton1Clicked: {
			const result = appDB.importFromFile(importExportFilename);
			displayResultMessage(result);
		}
	}

	function confirmImport(message: string) {
		importConfirmDialog.title = qsTr("Proceed with action?");
		importConfirmDialog.message = message;
		importConfirmDialog.show(-1);
	}

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

	function tryToOpenFile(fileName: string) {
		importExportFilename = fileName;
		if (importMessageDialog === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPImportMessageBox.qml", Qt.Asynchronous);

				function finishCreation() {
					importMessageDialog = component.createObject(contentItem, { parentPage: homePage });
					importMessageDialog.init(fileName);
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		else
			importMessageDialog.init(importExportFilename);
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

	function displayResultMessage(result: int) {
		var message;
		switch (result)
		{
			case  3: message = qsTr("Saved successfully to"); break;
			case  2: message = qsTr("Export successfully"); break;
			case  1: return; //Wait for user to confirm whether they will really import from the file
			case  0: message = qsTr("Import was successfull"); break;
			case -1: message = qsTr("Failed to open file"); break;
			case -2: message = qsTr("Error"); importExportFilename = qsTr("File type not recognized"); break;
			case -3: message = qsTr("Error"); importExportFilename = qsTr("File is formatted wrongly or is corrupted"); break;
			case -4: message = qsTr("Nothing to be done"); importExportFilename = qsTr("File had already been imported"); break;
			case -5:
				message = qsTr("No mesocycle to import into");
				importExportFilename = qsTr("Either create a new training plan or import from a complete program file");
			break;
			case -11: message = qsTr("Export failed"); break;
			case -12: importExportFilename = qsTr("Saving canceled");  break;
			case -6:
				message = qsTr("Nothing to save.");
				importExportFilename = qsTr("Only exercises that do not come by default with the app can be exported");
			break;
			case -10: message = qsTr("Something went wrong"); break;
		}
		activityFinishedTip.title = message;
		activityFinishedTip.message = importExportFilename;
		activityFinishedTip.showTimed(5000, 0);
	}

	function activityResultMessage(requestCode: int, resultCode: int) {
		console.log("****** requestCode: ", requestCode, "    resultCode: ", resultCode);
		var result;
		switch (resultCode) {
			case -1: result = 2; break;
			case 0: result = -11; break;
			default: result = -10; break;
		}
		displayResultMessage(result);
	}
} //ApplicationWindow
