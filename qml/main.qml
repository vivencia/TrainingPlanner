import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
	id: mainwindow
	objectName: "mainWindow"
	width: 300
	height: 640
	visible: true
	title: "Training Planner"
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint | Qt.WA_KeepScreenOn : Qt.Window

	signal backButtonPressed();
	signal mainMenuOpened();
	signal mainMenuClosed();

	readonly property string lightIconFolder: "white/"
	readonly property string darkIconFolder: "black/"
	readonly property int windowWidth: width
	readonly property int windowHeight: contentItem.height

	property var importMessageDialog: null
	property var importOpenDialog: null
	property string importExportFilename

	Component.onCompleted: {
		if (Qt.platform.os === "android") {
			contentItem.Keys.pressed.connect( function(event) {
				if (event.key === Qt.Key_Back) {
					event.accepted = true;
					if (stackView.depth >= 2)
						stackView.pop();
					else
						close();
				}
			});
		}
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

			onClicked: stackView.pop(stackView.get(0));
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

			onClicked: {
				function mesoCalendarOK()
				{
					appDB.databaseReady.disconnect(mesoCalendarOK);
					appDB.getTrainingDay(new Date());
				}

				if (mesoCalendarModel.count > 0)
					mesoCalendarOK();
				else {
					appDB.databaseReady.connect(mesoCalendarOK);
					appDB.getMesoCalendar(false);
				}
			} //onClicked
		} //TabButton
	} //footer

	function init() {
		homePage.setViewModel();
		btnWorkout.enabled = Qt.binding(function() {
			return stackView.depth === 1 && mesocyclesModel.mesoThatHasDate(new Date()) === mesocyclesModel.currentRow;
		});
		if (AppSettings.firstTime) {
			AppSettings.firstTime = false;
			stackView.push("SettingsPage.qml");
		}
	}

	function popFromStack(page: Item) {
		stackView.pop(page, StackView.Immediate);
	}

	function pushOntoStack(page: Item) {
		stackView.push(page);
	}

	function stackViewPushExistingPage(page: Item) {
		stackView.replace(stackView.currentItem, page);
	}

	function createShortCut(label: string, object: Item, clickid: int) {
		mainMenu.createShortCut(label, object, clickid);
	}

	TPBalloonTip {
		id: importConfirmDialog
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"import.png"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")

		onButton1Clicked: appDB.importFromFile(importExportFilename);
	}

	function confirmImport(message: string) {
		importConfirmDialog.title = qsTr("Proceed with action?");
		importConfirmDialog.message = message;
		importConfirmDialog.show(-1);
	}

	function chooseFileToImport() {
		if (importOpenDialog === null) {
			function createMessageBox() {
				var component = Qt.createComponent("TPImportDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					importOpenDialog = component.createObject(contentItem, {});
					importOpenDialog.open();
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		else
			importOpenDialog.open();
	}

	function tryToOpenFile(fileName: string) {
		importExportFilename = fileName;
		if (importMessageDialog === null) {
			function createMessageBox() {
				var component = Qt.createComponent("ImportMessageBox.qml", Qt.Asynchronous);

				function finishCreation() {
					importMessageDialog = component.createObject(contentItem, {});
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
		id: activityFinishedTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"import.png"
		button1Text: "OK"
	}

	function displayResultMessage(result: int) {
		var message;
		switch (result)
		{
			case  1: return; //Wait for user to confirm whether they will really import from the file
			case  0: message = qsTr("Import was successfull"); break;
			case -1: message = qsTr("Failed to open file"); break;
			case -2: message = qsTr("File type not recognized"); break;
			case -3: message = qsTr("File is formatted wrongly or is corrupted"); break;
			case -4: message = qsTr("Export successfully"); break;
			case -5: message = qsTr("Export failed"); break;
			case -6: message = qsTr("Something went wrong"); break;
		}
		activityFinishedTip.title = message;
		activityFinishedTip.message = importExportFilename;
		activityFinishedTip.showTimed(5000, 0);
	}

	function activityResultMessage(requestCode: int, resultCode: int) {
		console.log("****** requestCode: ", requestCode, "    resultCode: ", resultCode);
		var result;
		switch (resultCode) {
			case -1: result = -4; break;
			case 0: result = -5; break;
			default: result = -6; break;
		}
		displayResultMessage(result);
	}
} //ApplicationWindow
