import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

import "../"
import "../inexportMethods.js" as INEX
import "../ExercisesAndSets"
import "../TPWidgets"

import com.vivenciasoftware.qmlcomponents

TPPage {
	id: exercisesPage
	objectName: "exercisesPage"

	required property QmlItemManager itemManager
	property string strMediaPath
	property bool bCanEdit: false
	property bool bNew: false
	property bool bEdit: false
	property bool bChooseButtonEnabled: true
	property var imageViewer: null
	property var videoViewer: null

	signal exerciseChosen()

	property TPFloatingMenuBar imexportMenu: null
	readonly property bool bExportEnabled: !bChooseButtonEnabled

	onPageActivated: exercisesList.simulateMouseClick(0, true);

	onBExportEnabledChanged: {
		if (imexportMenu) {
			imexportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				imexportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	header: ToolBar {
		id: bottomPane
		width: parent.width
		height: parent.height * 0.5
		spacing: 0
		padding: 0

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

		ColumnLayout {
			width: parent.width
			height: parent.height
			spacing: 0

			ExercisesListView {
				id: exercisesList
				canDoMultipleSelection: bChooseButtonEnabled
				Layout.fillWidth: true
				Layout.topMargin: 0
				Layout.alignment: Qt.AlignTop
				Layout.rightMargin: 5
				Layout.fillHeight: true
				Layout.leftMargin: 5
				Layout.bottomMargin: 5

				onExerciseEntrySelected: (index, multipleSelection) => {
					if (multipleSelection === 2) return;
					txtExerciseName.text = exercisesModel.get(index, 1);
					txtExerciseSubName.text = exercisesModel.get(index, 2);
					txtMuscularGroup.text = exercisesModel.get(index, 3);
					txtNSets.text = exercisesModel.get(index, 4);
					txtNReps.text = exercisesModel.get(index, 5);
					txtNWeight.text = exercisesModel.get(index, 6);
					strMediaPath = exercisesModel.get(index, 8);
					displaySelectedMedia();
				}

				onItemDoubleClicked: {
					if (btnAddExercise.enabled)
						chooseExercise();
				}
			}

			RowLayout {
				id: toolbarExercises
				Layout.fillWidth: true
				Layout.maximumHeight: 30
				Layout.leftMargin: 5
				Layout.bottomMargin: 5
				spacing: 5

				readonly property int buttonWidth: Math.ceil(exercisesPage.width/5.5)

				TPButton {
					id:btnNewExercise
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
							strMediaPath = "qrc:/images/no_image.jpg";
							exercisesList.enabled = false;
							text = qsTr("Cancel");
							exercisesModel.appendRow();
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
							//if (!bJustSaved)
							//	exercisesList.itemClicked(exercisesModel.currentRow, true);
						}
					}
				} //btnEditExercise

				TPButton {
					id: btnSaveExercise
					text: qsTr("Save")
					enabled: (bNew && txtExerciseName.length > 5) || (bEdit && txtExerciseName.length > 5)
					fixedSize: true
					width: toolbarExercises.buttonWidth
					height: 25
					rounded: false
					flat: false

					onClicked: {
						appDB.saveExercises();
						if (bNew) {
							btnNewExercise.clicked();
							//exercisesList.simulateMouseClick(exercisesModel.count - 1);
						}
						else if (bEdit)
							btnEditExercise.clicked();
					}
				} //btnSaveExercise

				TPButton {
					id: btnAddExercise
					enabled: bChooseButtonEnabled && !bCanEdit && exercisesModel.currentRow >= 0
					text: qsTr("Add")
					fixedSize: true
					width: toolbarExercises.buttonWidth
					height: 25
					rounded: false
					flat: false

					onClicked: chooseExercise();
				} //btnAddExercise

				TPButton {
					id: btnImExport
					text: qsTr("In/Export")
					enabled: !btnSaveExercise.enabled
					visible: !bChooseButtonEnabled
					fixedSize: true
					width: toolbarExercises.buttonWidth
					height: 25
					rounded: false
					flat: false

					onClicked: INEX.showInExMenu(exercisesPage, true);
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
			width: exercisesPage.width

			Label {
				text: exercisesModel.columnLabel(1)
				color: AppSettings.fontColor
				font.pointSize: AppSettings.fontSizeText
				font.bold: true
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
				onEditingFinished: exercisesModel.setMainName(text);
			}

			Label {
				text: exercisesModel.columnLabel(2)
				color: AppSettings.fontColor
				font.pointSize: AppSettings.fontSizeText
				font.bold: true
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
				onEditingFinished: exercisesModel.setSubName(text);
			}

			Label {
				text: exercisesModel.columnLabel(3)
				color: AppSettings.fontColor
				font.pointSize: AppSettings.fontSizeText
				font.bold: true
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

				onEnterOrReturnKeyPressed: txtNSets.forceActiveFocus();
				onEditingFinished: exercisesModel.setMuscularGroup(text);
			}

			Label {
				id: lblDefaults
				text: qsTr("Exercise details:")
				color: AppSettings.fontColor
				font.pointSize: AppSettings.fontSizeText
				font.bold: true
				Layout.minimumWidth: parent.width - 20
				Layout.maximumWidth: parent.width - 20
				Layout.leftMargin: 5
				Layout.topMargin: 10
				z: 1

				TPButton {
					imageSource: paneExerciseDefaults.shown ? "fold-up.png" : "fold-down.png"
					imageSize: 20
					height: 20
					width: 20

					anchors {
						left: lblDefaults.right
						leftMargin: -20
						verticalCenter: lblDefaults.verticalCenter
					}
					onClicked: paneExerciseDefaults.shown = !paneExerciseDefaults.shown
				}
			} //Label

			Frame {
				id: paneExerciseDefaults
				property bool shown: true
				visible: height > 0
				height: shown ? implicitHeight : 0
				Behavior on height {
					NumberAnimation {
						easing.type: Easing.InOutBack
					}
				}
				clip: true
				padding: 0
				z: 0

				background: Rectangle {
					border.color: AppSettings.fontColor
					color: "transparent"
					radius: 6
				}

				implicitHeight: colLayout.implicitHeight + 20
				implicitWidth: layoutMain.width - 20
				Layout.leftMargin: 5

				ColumnLayout {
					id: colLayout
					anchors.fill: parent
					anchors.bottomMargin: 10
					spacing: 2
					enabled: bCanEdit

					SetInputField {
						id: txtNSets
						type: SetInputField.Type.SetType
						availableWidth: parent.width*0.6
						alternativeLabels: ["", "", "", exercisesModel.columnLabel(4)]
						backColor: "transparent"
						borderColor: "transparent"
						labelColor: AppSettings.fontColor
						Layout.alignment: Qt.AlignCenter

						onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
						onValueChanged: (str) => exercisesModel.setSetsNumber(str);
					}

					SetInputField {
						id: txtNReps
						type: SetInputField.Type.RepType
						availableWidth: parent.width*0.6
						alternativeLabels: ["", exercisesModel.columnLabel(5)]
						backColor: "transparent"
						borderColor: "transparent"
						labelColor: AppSettings.fontColor
						Layout.alignment: Qt.AlignCenter

						onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
						onValueChanged: (str) => exercisesModel.setRepsNumber(str);
					}

					SetInputField {
						id: txtNWeight
						type: SetInputField.Type.WeightType
						availableWidth: parent.width*0.6
						alternativeLabels: [exercisesModel.columnLabel(6), ""]
						backColor: "transparent"
						borderColor: "transparent"
						labelColor: AppSettings.fontColor
						Layout.alignment: Qt.AlignCenter

						onEnterOrReturnKeyPressed: btnChooseMediaFromDevice.forceActiveFocus();
						onValueChanged: (str) => exercisesModel.setWeight(str);
					}
				} // ColumnLayout
			} //Pane

			Label {
				text: exercisesModel.columnLabel(8)
				color: AppSettings.fontColor
				font.pointSize: AppSettings.fontSizeText
				font.bold: true
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

	TPBalloonTip {
		id: exportTypeTip
		title: bShare ? qsTr("Share custom exercises?") : qsTr("Export custom exercises to file?")
		imageSource:  "export"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		parentPage: exercisesPage

		onButton1Clicked: itemManager.exportExercises(bShare);

		property bool bShare: false

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}

	FileDialog {
		id: fileDialog
		title: qsTr("Please choose a media file");

		onAccepted: {
			strMediaPath = appUtils.getCorrectPath(currentFile);
			close();
			displaySelectedMedia();
		}
	}

	function chooseExercise() {
		exerciseChosen();
		mainwindow.popFromStack();
	}

	function displaySelectedMedia() {
		if (strMediaPath.length < 5)
			return;
		var mediaType = appUtils.getFileType(strMediaPath);
		if ( mediaType === 1) { //video
			if (imageViewer !== null) {
				imageViewer.destroy();
				imageViewer = null;
			}
			if (videoViewer === null)
				generateObject(1);
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
				generateObject(0);
			else
				imageViewer.mediaSource = strMediaPath;
		}
	}

	function generateObject(obj) {
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
} // Page
