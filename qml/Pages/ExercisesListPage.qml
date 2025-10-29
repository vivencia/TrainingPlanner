import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

import "../"
import "../ExercisesAndSets"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: exercisesPage
	imageSource: ":/images/backgrounds/exercises_backimage.jpg"
	backgroundOpacity: 0.8
	objectName: "exercisesPage"

	required property ExercisesListManager exercisesManager
	property bool bCanEdit: false
	property bool bNew: false
	property bool bEdit: false
	property bool bChooseButtonEnabled: true
	property ImageViewer imageViewer: null
	property VideoPlayer videoViewer: null

	signal exerciseChosen()

	onPageActivated: if (exercisesModel.count > 0) exercisesList.simulateMouseClick(0, true);

	ExercisesListView {
		id: exercisesList
		parentPage: exercisesPage
		canDoMultipleSelection: bChooseButtonEnabled
		height: parent.height * 0.5

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onExerciseEntrySelected: (index, multipleSelection) => {
			txtExerciseName.text = exercisesModel.mainName(index);
			txtExerciseSubName.text = exercisesModel.subName(index);
			txtMuscularGroup.text = exercisesModel.muscularGroup(index);
			displaySelectedMedia();
		}

		onItemDoubleClicked: {
			if (btnAddExercise.enabled)
				chooseExercise();
		}
	}

	Row {
		id: toolbarExercises
		spacing: 0
		height: appSettings.itemDefaultHeight

		anchors {
			top: exercisesList.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
		readonly property int buttonWidth: (parent.width - 10) * 0.25

		TPButton {
			id: btnNewExercise
			text: qsTr("New")
			enabled: !bEdit
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: {
				if (!bNew) {
					bNew = true;
					bCanEdit = true;
					scrollExercises.ScrollBar.vertical.setPosition(0);
					txtExerciseName.forceActiveFocus();
					txtExerciseName.clear();
					txtExerciseSubName.clear();
					txtMuscularGroup.clear();
					exercisesList.enabled = false;
					text = qsTr("Cancel");
					exercisesModel.newExercise();
				}
				else {
					exercisesModel.removeRow(exercisesModel.currentRow);
					bNew = false;
					bCanEdit = false;
					exercisesList.enabled = true;
					text = qsTr("New");
				}
			}
		} //btnNewExercise

		TPButton {
			id: btnEditExercise
			text: qsTr("Edit")
			enabled: !bNew && exercisesModel.currentRow >= 0
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: {
				if (!bEdit) {
					bCanEdit = true;
					bEdit = true;
					scrollExercises.ScrollBar.vertical.setPosition(0);
					txtExerciseName.forceActiveFocus();
					exercisesList.enabled = false;
					text = qsTr("Cancel");
				}
				else {
					bCanEdit = false;
					bEdit = false;
					exercisesList.enabled = true;
					text = qsTr("Edit");
				}
			}
		} //btnEditExercise

		TPButton {
			id: btnAddExercise
			enabled: bChooseButtonEnabled && !bCanEdit && exercisesModel.currentRow >= 0
			text: qsTr("Add")
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: chooseExercise();
		} //btnAddExercise

		TPButton {
			id: btnImExport
			text: qsTr("In/Export")
			visible: !bChooseButtonEnabled
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: showInExMenu();
		} // btnImExport

	} // Row

	ScrollView {
		id: scrollExercises
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: layoutMain.implicitHeight
		padding: 2

		anchors {
			top: toolbarExercises.bottom
			topMargin: 15
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
			bottomMargin: 5
		}

		ColumnLayout {
			id: layoutMain
			spacing: 10
			anchors.fill: parent

			TPLabel {
				text: exercisesModel.exerciseNameLabel
			}
			TPTextInput {
				id: txtExerciseName
				readOnly: !bCanEdit
				font.italic: bCanEdit
				Layout.fillWidth: true
				Layout.rightMargin: 10

				onEnterOrReturnKeyPressed: txtMuscularGroup.forceActiveFocus();
				onEditingFinished: exercisesModel.setMainName(exercisesModel.currentRow, text);
			}

			TPLabel {
				text: exercisesModel.exerciseSpecificsLabel
			}

			TPTextInput {
				id: txtExerciseSubName
				readOnly: !bCanEdit
				font.italic: bCanEdit
				Layout.fillWidth: true
				Layout.rightMargin: 10

				onEnterOrReturnKeyPressed: txtExerciseSubName.forceActiveFocus();
				onEditingFinished: exercisesModel.setSubName(exercisesModel.currentRow, text);
			}

			TPLabel {
				text: exercisesModel.muscularGroupsLabel
			}
			TPTextInput {
				id: txtMuscularGroup
				readOnly: !bCanEdit
				font.italic: bCanEdit
				Layout.fillWidth: true
				Layout.rightMargin: 10
				Layout.minimumHeight: 30
				Layout.maximumHeight: 80

				onEditingFinished: exercisesModel.setMuscularGroup(exercisesModel.currentRow, text);
			}

			TPLabel {
				text: exercisesModel.mediaLabel
			}

			TPButton {
				id: btnChooseMediaFromDevice
				text: qsTr("Choose media")
				autoSize: true
				rounded: false
				onClicked: fileDialog.open();
				Layout.alignment: Qt.AlignCenter
				enabled: bNew || bEdit
			}
		} // ColumnLayout
	} // ScrollView

	FileDialog {
		id: fileDialog
		title: qsTr("Please choose a media file");
		nameFilters: [qsTr("Videos") + " (*.mp4 *.mkv)", qsTr("Images") + " (*.jpg *.jpeg *.png)"]
		options: FileDialog.ReadOnly
		currentFolder: StandardPaths.standardLocations(StandardPaths.MoviesLocation)[0]
		fileMode: FileDialog.OpenFile

		onAccepted: {
			const mediapath = appUtils.getCorrectPath(currentFile);
			exercisesModel.setMediaPath(exercisesModel.currentRow, mediapath);
			displaySelectedMedia(mediapath);
		}
	}

	function chooseExercise(): void {
		exerciseChosen();
		itemManager.appPagesModel.prevPage();
	}

	function displaySelectedMedia(strMediaPath: string): void {
		const mediaType = appUtils.getFileType(strMediaPath);
		if (mediaType === 1) { //video
			if (imageViewer !== null) {
				imageViewer.destroy();
				imageViewer = null;
			}
			if (videoViewer === null)
				generateObject(1, strMediaPath);
			else
				videoViewer.mediaSource = strMediaPath;
		}
		else {
			if (mediaType === -1) //unknown
				strMediaPath = "qrc:/images/no_image.jpg";
			if (videoViewer !== null) {
				videoViewer.destroy();
				videoViewer = null;
			}
			if (imageViewer === null)
				generateObject(0, strMediaPath);
			else
				imageViewer.mediaSource = strMediaPath;
		}
	}

	function generateObject(obj: int, strMediaPath: string): void {
		let component = Qt.createComponent(obj === 0 ? "../ImageViewer.qml" : "../VideoPlayer.qml", Qt.Asynchronous);
		function finishCreation(Obj) {
			if (Obj === 0)
				imageViewer = component.createObject(layoutMain, { mediaSource:strMediaPath });
			else
				videoViewer = component.createObject(layoutMain, { mediaSource:strMediaPath });
		}

		function checkStatus() {
			if (component.status === Component.Ready)
				finishCreation(obj);
		}
		if (component.status === Component.Ready)
			finishCreation(obj);
		else
			component.statusChanged.connect(checkStatus);
	}

	property TPFloatingMenuBar imExportMenu: null
	readonly property bool bExportEnabled: !bChooseButtonEnabled
	onBExportEnabledChanged: {
		if (imExportMenu) {
			imExportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				imExportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	function showInExMenu(): void {
		if (imExportMenu === null) {
			var imExportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			imExportMenu = imExportMenuComponent.createObject(exercisesPage, { parentPage: exercisesPage });
			imExportMenu.addEntry(qsTr("Import"), "import.png", 0, true);
			imExportMenu.addEntry(qsTr("Export"), "save-day.png", 1, true);
			if (Qt.platform.os === "android")
				imExportMenu.addEntry(qsTr("Share"), "export.png", 2, true);
			imExportMenu.menuEntrySelected.connect(functionselectedMenuOption);
		}
		imExportMenu.show2(btnImExport, 0);
	}

	function selectedMenuOption(menuid: int): void {
		switch (menuid) {
			case 0: exercisesManager.importExercises(); break;
			case 1: exportTypeTip.init(false); break;
			case 2: exportTypeTip.init(true); break;
		}
	}

	TPBalloonTip {
		id: exportTypeTip
		title: bShare ? qsTr("Share custom exercises?") : qsTr("Export custom exercises to file?")
		imageSource:  "export"
		parentPage: exercisesPage
		closeButtonVisible: true

		onButton1Clicked: exercisesManager.exportExercises(bShare);

		property bool bShare: false

		function init(share: bool): void {
			bShare = share;
			show(-1);
		}
	}
} // Page
