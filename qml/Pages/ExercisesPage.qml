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
	objectName: "exercisesPage"

	required property ExercisesListManager exercisesManager
	property bool bCanEdit: false
	property bool bNew: false
	property bool bEdit: false
	property bool bChooseButtonEnabled: true
	property var imageViewer: null
	property var videoViewer: null

	signal exerciseChosen()

	onPageActivated: if (exercisesModel.count > 0) exercisesList.simulateMouseClick(0, true);

	header: TPToolBar {
		id: bottomPane
		height: parent.height * 0.5

		ColumnLayout {
			anchors.fill: parent
			anchors.leftMargin: 5
			anchors.rightMargin: 5
			anchors.topMargin: 5
			spacing: 0

			ExercisesListView {
				id: exercisesList
				parentPage: exercisesPage
				canDoMultipleSelection: bChooseButtonEnabled
				Layout.alignment: Qt.AlignTop
				Layout.fillWidth: true
				Layout.fillHeight: true
				Layout.bottomMargin: 5

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
				Layout.fillWidth: true
				Layout.maximumHeight: 30
				spacing: 0

				readonly property int buttonWidth: parent.width*0.25

				TPButton {
					id: btnNewExercise
					text: qsTr("New")
					enabled: !bEdit
					fixedSize: true
					width: toolbarExercises.buttonWidth
					height: 25
					rounded: false
					flat: false

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
					fixedSize: true
					width: toolbarExercises.buttonWidth
					height: 25
					rounded: false
					flat: false

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
					fixedSize: true
					autoResize: true
					width: toolbarExercises.buttonWidth
					height: 25
					rounded: false
					flat: false

					onClicked: chooseExercise();
				} //btnAddExercise

				TPButton {
					id: btnImExport
					text: qsTr("In/Export")
					visible: !bChooseButtonEnabled
					fixedSize: true
					autoResize: true
					width: toolbarExercises.buttonWidth
					height: 25
					rounded: false
					flat: false

					onClicked: showInExMenu();
				} // btnImExport

			} // Row
		} //ColumnLayout
	} // header

	ScrollView {
		id: scrollExercises
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: layoutMain.implicitHeight
		padding: 2

		ColumnLayout {
			id: layoutMain
			width: appSettings.pageWidth

			TPLabel {
				text: exercisesModel.exerciseNameLabel
				Layout.leftMargin: 5
				Layout.topMargin: 10
			}
			TPTextInput {
				id: txtExerciseName
				readOnly: !bCanEdit
				font.italic: bCanEdit
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 20

				onEnterOrReturnKeyPressed: txtMuscularGroup.forceActiveFocus();
				onEditingFinished: exercisesModel.setMainName(exercisesModel.currentRow, text);
			}

			TPLabel {
				text: exercisesModel.exerciseSpecificsLabel
				Layout.leftMargin: 5
			}

			TPTextInput {
				id: txtExerciseSubName
				readOnly: !bCanEdit
				font.italic: bCanEdit
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 20

				onEnterOrReturnKeyPressed: txtExerciseSubName.forceActiveFocus();
				onEditingFinished: exercisesModel.setSubName(exercisesModel.currentRow, text);
			}

			TPLabel {
				text: exercisesModel.muscularGroupsLabel
				Layout.leftMargin: 5
				Layout.topMargin: 10
			}
			TPTextInput {
				id: txtMuscularGroup
				readOnly: !bCanEdit
				font.italic: bCanEdit
				Layout.fillWidth: true
				Layout.rightMargin: 20
				Layout.leftMargin: 10

				onEditingFinished: exercisesModel.setMuscularGroup(exercisesModel.currentRow, text);
			}

			TPLabel {
				text: exercisesModel.mediaLabel
				Layout.bottomMargin: 10
				Layout.topMargin: 10
				Layout.leftMargin: 5
			}

			TPButton {
				id: btnChooseMediaFromDevice
				text: qsTr("Choose media")
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
		mainwindow.popFromStack();
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
		var component = Qt.createComponent(obj === 0 ? "../ImageViewer.qml" : "../VideoPlayer.qml", Qt.Asynchronous);
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
