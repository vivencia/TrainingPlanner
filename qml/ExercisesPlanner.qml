import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import com.vivenciasoftware.qmlcomponents

Page {
	id: pagePlanner
	objectName: "exercisesPlanner"
	width: windowWidth
	height: windowHeight

	required property int mesoId
	required property int mesoIdx

	property bool bEnableMultipleSelection: false
	property bool bShowSimpleExercisesList: false
	property var itemThatRequestedSimpleList: null

	property alias currentPage: splitView.currentItem

	Keys.onPressed: (event) => {
		if (event.key === Qt.Key_Back) {
			event.accepted = true;
			if (exercisesPane.visible)
				exercisesPane.visible = false;
			else
				pagePlanner.StackView.pop();
		}
	}

	SwipeView {
		id: splitView
		objectName: "splitSwipeView"
		currentIndex: -1
		height: parent.height
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		interactive: !exercisesPane.shown

		onCurrentIndexChanged: currentItem.init();
	} //SwipeView

	PageIndicator {
		id: indicator
		count: splitView.count
		currentIndex: splitView.currentIndex
		visible: !exercisesPane.shown
		height: 20
		anchors {
			bottom: parent.bottom
			horizontalCenter: parent.horizontalCenter
		}
	}

	footer: ToolBar {
		id: splitToolBar
		width: parent.width
		height: 55
		visible: !bShowSimpleExercisesList

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			id: btnSave
			text: qsTr("Save")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"save-day.png"
			textUnderIcon: true
			enabled: splitView.currentIndex >= 0 ? currentPage.splitModel.modified : false
			fixedSize: true
			width: 55
			height: btnAddExercise.height
			anchors {
				left: parent.left
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				appDB.updateMesoSplitComplete(currentPage.splitModel);
				requestSimpleExercisesList(null, false, false);
			}
		}

		TPButton {
			id: btnClearPlan
			text: qsTr("Clear")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"clear.png"
			textUnderIcon: true
			enabled: splitView.currentIndex >= 0 ? currentPage.splitModel.count > 1 : false
			fixedSize: true
			width: 55
			height: btnAddExercise.height
			anchors {
				left: btnSave.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				currentPage.splitModel.clear();
				currentPage.appendNewExerciseToDivision();
				requestSimpleExercisesList(null, false);
			}
		}

		TPButton {
			id: btnSwapPlan
			text: splitView.currentIndex >= 0 ? currentPage.splitModel.splitLetter + " <-> " + currentPage.swappableLetter : "A <-> B"
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"swap.png"
			textUnderIcon: true
			visible: currentPage ? currentPage.bCanSwapPlan : false
			fixedSize: true
			width: 55
			height: btnAddExercise.height
			anchors {
				left: btnClearPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: appDB.swapMesoPlans(currentPage.splitModel.splitLetter, currentPage.swappableLetter);
		}

		TPButton {
			id: btnInExport
			text: qsTr("In/Ex")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"import-export.png"
			textUnderIcon: true
			visible: splitView.currentIndex >= 0 ? currentPage.splitModel.count > 1 : false
			fixedSize: true
			width: 55
			height: btnAddExercise.height
			anchors {
				left: btnSwapPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			property var inexportMenu: null

			onClicked: {
				if (inexportMenu === null) {
					var inexportMenuComponent = Qt.createComponent("TPFloatingMenuBar.qml");
					inexportMenu = inexportMenuComponent.createObject(pagePlanner, {});
					inexportMenu.addEntry(qsTr("Export"), "export.png", 0);
					inexportMenu.addEntry(qsTr("Import"), "import.png", 1);
					inexportMenu.menuEntrySelected.connect(this.selectedMenuOption);
				}
				inexportMenu.show(btnInExport, 0);
			}

			function selectedMenuOption(menuid: int) {
				if (menuid === 0)
					exportTypeTip.show(-1);
				else
					importDialog.open();
			}
		}

		TPButton {
			id: btnAddExercise
			text: qsTr("+ Exercise")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"exercises-add.png"
			textUnderIcon: true
			fixedSize: true
			width: 70
			anchors {
				right: parent.right
				rightMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: currentPage.appendNewExerciseToDivision();
		} //btnAddExercise
	}

	SimpleExercisesListPanel {
		id: exercisesPane
	}

	Component.onCompleted: {
		function insertSplitPage(page, idx) {
			if (idx < 6)
				splitView.insertItem(idx, page);
		}

		itemManager.pageReady.connect(insertSplitPage);
		appDB.getCompleteMesoSplit();
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		itemThatRequestedSimpleList = object;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
	}

	function hideSimpleExerciseList() {
		exercisesPane.shown = false;
	}

	TPBalloonTip {
		id: exportTypeTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"export.png"
		message: qsTr("What do you want to export?")
		button1Text: qsTr("Entire plan")
		button2Text: qsTr("Just this split")
		checkBoxText: qsTr("Human readable?")

		onButton1Clicked: exportDialog.init(0, checkBoxChecked);
		onButton2Clicked: exportDialog.init(1, checkBoxChecked);
	}

	TPBalloonTip {
		id: exportTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"export.png"
		button1Text: "OK"

		function init(msg: string) {
			message = msg;
			showTimed(5000, 0);
		}
	}

	TPBalloonTip {
		id: importTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"import.png"
		button1Text: "OK"

		function init(header: string, msg: string) {
			title = header;
			message = msg;
			showTimed(5000, 0);
		}
	}

	FileDialog {
		id: exportDialog
		title: qsTr("Choose the folder and filename to export to")
		currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
		fileMode: FileDialog.SaveFile

		property int _opt
		property bool _bfancyFormat

		onAccepted: {
			var result;
			if (_opt === 0) {
				for (var i = 0; i < splitView.count; ++i)
					result = appDB.exportToFile(splitView.itemAt(i).splitModel, currentFile, _bfancyFormat);
			}
			else
				result = appDB.exportToFile(currentPage.splitModel, currentFile, _bfancyFormat);
			exportTip.init(result ? qsTr("Meso plan successfully exported") : qsTr("Failed to export meso plan"));
			close();
		}

		function init(opt: int, fancy: bool) {
			var suggestedName;
			_opt = opt;
			_bfancyFormat = fancy;
			if (opt === 0)
				suggestedName = qsTr(" - Exercises Plan.tp")
			else
				suggestedName = qsTr(" - Exercises Plan - Split ") + currentPage.splitModel.splitLetter + ".tp";

			currentFile = mesocyclesModel.get(mesoIdx, 1) + suggestedName;
			open();
		}
	}

	FileDialog {
		id: importDialog
		title: qsTr("Choose the file to import from")
		currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
		fileMode: FileDialog.OpenFile

		property int _opt
		property bool _bfancyFormat

		onAccepted: {
			const result = appDB.importFromFile(currentFile);
			var message;
			switch (result)
			{
				case  0: message = qsTr("Import was successfull"); break;
				case -1: message = qsTr("Failed to open file"); break;
				case -2: message = qsTr("File type not recognized"); break;
				case -3: message = qsTr("File is formatted wrongly or is corrupted"); break;
			}
			importTip.init(message, currentFile);
			close();
		}
	}
} //Page
