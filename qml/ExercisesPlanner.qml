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
	property var navButtons: null
	property var inexportMenu: null

	property alias currentPage: splitView.currentItem
	readonly property bool bExportEnabled: splitView.currentIndex >= 0 ? currentPage.splitModel.count > 1 : false

	onBExportEnabledChanged: {
		if (inexportMenu) {
			inexportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				inexportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	Keys.onBackPressed: (event) => {
		event.accepted = true;
		if (exercisesPane.visible)
			requestSimpleExercisesList(null, false, false);
		else
			pagePlanner.StackView.pop();
	}

	SwipeView {
		id: splitView
		objectName: "splitSwipeView"
		currentIndex: -1
		interactive: !exercisesPane.shown
		height: parent.height
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

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
			fixedSize: true
			width: 55
			height: btnAddExercise.height
			anchors {
				left: btnSwapPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				if (inexportMenu === null) {
					var inexportMenuComponent = Qt.createComponent("TPFloatingMenuBar.qml");
					inexportMenu = inexportMenuComponent.createObject(pagePlanner, {});
					inexportMenu.addEntry(qsTr("Import"), "import.png", 0);
					inexportMenu.addEntry(qsTr("Save"), "save-day.png", 1);
					if (Qt.platform.os === "android")
						inexportMenu.addEntry(qsTr("Export"), "export.png", 2);
					inexportMenu.menuEntrySelected.connect(this.selectedMenuOption);
				}
				inexportMenu.show(btnInExport, 0);
			}

			function selectedMenuOption(menuid: int) {
				switch (menuid) {
					case 0: importDialog.open(); break;
					case 1: exportTypeTip.init(true); break;
					case 2: exportTypeTip.init(false); break;
				}
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

			onClicked: {
				if (navButtons === null)
					createNavButtons();
				currentPage.appendNewExerciseToDivision();
			}
		} //btnAddExercise
	}

	SimpleExercisesListPanel {
		id: exercisesPane
	}

	TPImportDialog {
		id: importDialog
	}

	Component.onCompleted: {
		appDB.getCompleteMesoSplit();
		pagePlanner.StackView.activating.connect(pageActivation);
		pagePlanner.StackView.onDeactivating.connect(pageDeActivation);
	}

	function pageActivation() {
		if (navButtons)
			navButtons.visible = true;
	}

	function pageDeActivation() {
		if (navButtons)
			navButtons.visible = false;
	}

	function createNavButtons() {
		if (navButtons === null) {
			var component = Qt.createComponent("PageScrollButtons.qml", Qt.Asynchronous);

			function finishCreation() {
				navButtons = component.createObject(pagePlanner, {});
				navButtons.scrollTo.connect(currentPage.setScrollBarPosition);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		itemThatRequestedSimpleList = object;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
		if (navButtons !== null) {
			if (visible)
				navButtons.hideButtons();
			else
				navButtons.showButtons();
		}
	}

	function hideSimpleExerciseList() {
		exercisesPane.shown = false;
		if (navButtons !== null)
			navButtons.showButtons();
	}

	function insertSplitPage(page: Item, idx: int) {
		splitView.insertItem(idx, page);
	}

	TPBalloonTip {
		id: exportTypeTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"export.png"
		message: saveOpt ? qsTr("What to you want to save?") : qsTr("What do you want to export?")
		button1Text: qsTr("Entire plan")
		button2Text: qsTr("Just this split")
		checkBoxText: qsTr("Human readable?")

		onButton1Clicked: {
			if (saveOpt)
				saveDialog.init(0, checkBoxChecked);
			else {
				const result = appDB.exportMesoSplit("X", checkBoxChecked);
				exportTip.init(result ? qsTr("Entire Meso plan successfully exported") : qsTr("Failed to export meso plan"));
			}
		}
		onButton2Clicked: {
			if (saveOpt)
				saveDialog.init(1, checkBoxChecked);
			else {
				const result = appDB.exportMesoSplit(currentPage.splitModel.splitLetter, checkBoxChecked);
				exportTip.init(result ? qsTr("Meso split plan successfully exported") : qsTr("Failed to export meso split plan"));
			}
		}

		property bool saveOpt: false

		function init(bSave: bool) {
			saveOpt = bSave;
			show(-1);
		}
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

	FileDialog {
		id: saveDialog
		title: qsTr("Choose the folder and filename to save to")
		currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
		fileMode: FileDialog.SaveFile

		property int _opt
		property bool _bfancyFormat

		onAccepted: {
			var result;
			if (_opt === 0) {
				for (var i = 0; i < splitView.count; ++i)
					//result = appDB.exportToFile(splitView.itemAt(i).splitModel, currentFile, _bfancyFormat);
					result = appDB.exportToFile(splitView.itemAt(i).splitModel, "/home/guilherme/Dokumente/tp/batista5.tp", _bfancyFormat);
			}
			else
				result = appDB.exportToFile(currentPage.splitModel, currentFile, _bfancyFormat);
			exportTip.init(result ? qsTr("Meso plan successfully saved") : qsTr("Failed to save meso plan"));
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
} //Page
