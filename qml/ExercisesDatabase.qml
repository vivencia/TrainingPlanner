import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import "jsfunctions.js" as JSF

Page {
	id: pageExercises

	property string strMediaPath

	property bool bCanEdit: false
	property bool bNew: false
	property bool bEdit: false
	property bool bChooseButtonEnabled: false
	property bool bJustSaved: false
	property var imageViewer: null
	property var videoViewer: null

	signal exerciseChosen(string strName1, string strName2, int nSets, real nReps, real nWeight, bool bAdd)

	background: Rectangle {
		color: primaryDarkColor
		opacity: 0.7
		Image {
			anchors.fill: parent
			source: "qrc:/images/app_logo.png"
			fillMode: Image.PreserveAspectFit
			opacity: 0.6
		}
	}

	ScrollView {
		id: scrollExercises
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		padding: 2

		ColumnLayout {
			id: layoutMain
			width: scrollExercises.availableWidth

			Label {
				text: qsTr("Exercise:")
				color: "white"
				font.pixelSize: AppSettings.fontSizeText
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

				Keys.onReturnPressed: { //Alphanumeric keyboard
					txtExerciseSubName.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					txtExerciseSubName.forceActiveFocus();
				}
			}

			Label {
				text: qsTr("Specifics:")
				color: "white"
				font.pixelSize: AppSettings.fontSizeText
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

				Keys.onReturnPressed: { //Alphanumeric keyboard
					txtMuscularGroup.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					txtMuscularGroup.forceActiveFocus();
				}
			}

			Label {
				text: qsTr("Muscular Group:")
				color: "white"
				font.pixelSize: AppSettings.fontSizeText
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

				Keys.onReturnPressed: { //Alphanumeric keyboard
					btnSaveExercise.clicked();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					btnSaveExercise.clicked();
				}
			}

			Label {
				id: lblDefaults
				text: qsTr("Optional defaults")
				color: "white"
				font.pixelSize: AppSettings.fontSizeText
				font.bold: true
				Layout.minimumWidth: parent.width - 20
				Layout.maximumWidth: parent.width - 20
				Layout.leftMargin: 5
				z: 1

				Image {
					anchors.left: lblDefaults.right
					anchors.leftMargin: -20
					anchors.verticalCenter: lblDefaults.verticalCenter
					source: paneExerciseDefaults.shown ? "qrc:/images/"+darkIconFolder+"fold-up.png" : "qrc:/images/"+darkIconFolder+"fold-down.png"
					height: 20
					width: 20
					z: 0
				}

				MouseArea {
					anchors.fill: parent
					onClicked: paneExerciseDefaults.shown = !paneExerciseDefaults.shown
					z:2
				}
			} //Label

			Frame {
				id: paneExerciseDefaults
				property bool shown: false
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
					border.color: "white"
					radius: 6
				}

				implicitHeight: gridLayout.implicitHeight + 10
				implicitWidth: layoutMain.width - 20
				Layout.leftMargin: 5

				GridLayout {
					id: gridLayout
					anchors.fill: parent
					columns: 2
					rows: 3
					rowSpacing: 0
					columnSpacing: 0
					enabled: bCanEdit

					Label {
						text: qsTr("Sets: ")
						color: "white"
						font.pixelSize: AppSettings.fontSizeText
						font.bold: true
						Layout.row: 0
						Layout.column: 0
					}
					SpinBox {
						id: spinSets
						from: 0
						to: 9
						editable: true
						Layout.row: 0
						Layout.column: 1
						font.bold: true
						font.pixelSize: AppSettings.fontSizeText
					}

					Label {
						text: qsTr("Reps: ")
						color: "white"
						font.pixelSize: AppSettings.fontSizeText
						font.bold: true
						Layout.row: 1
						Layout.column: 0
					}
					SpinBox {
						id: spinReps
						from: 0
						to: 40
						editable: true
						Layout.row: 1
						Layout.column: 1
						font.bold: true
						font.pixelSize: AppSettings.fontSizeText
					}

					Label {
						text: qsTr("Weigth: ")
						color: "white"
						font.pixelSize: AppSettings.fontSizeText
						font.bold: true
						Layout.row: 2
						Layout.column: 0
					}
					SpinBox {
						id: spinWeight
						from: 1
						to: 999
						editable: true
						Layout.row: 2
						Layout.column: 1
						font.bold: true
						font.pixelSize: AppSettings.fontSizeText
					}
				} // GridLayout
			} //Pane

			Label {
				text: qsTr("Descriptive media:")
				color: "white"
				font.pixelSize: AppSettings.fontSizeText
				font.bold: true
				Layout.bottomMargin: 10
				Layout.topMargin: 10
				Layout.leftMargin: 5
			}

			ButtonFlat {
				id: btnChooseMediaFromDevice
				text: qsTr("File")
				onClicked: chooseMediaFromDevice();
				Layout.alignment: Qt.AlignCenter
			}
		} // ColumnLayout
	} // ScrollView

	function exerciseSelected(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath) {
		txtExerciseName.text = exerciseName;
		txtExerciseSubName.text = subName;
		txtMuscularGroup.text = muscularGroup;
		spinSets.value = sets;
		spinReps.value = reps;
		spinWeight.value = weight;
		strMediaPath = mediaPath;
		displaySelectedMedia();
	}

	footer: ToolBar {
		id: bottomPane
		width: parent.width
		height: parent.height * 0.5
		spacing: 0
		padding: 0
		background: Rectangle {
			opacity: 0.3
			color: paneBackgroundColor
		}

		ColumnLayout{
			width: parent.width
			height: parent.height
			spacing: 0

			ExercisesListView {
				id: exercisesList
				Layout.fillWidth: true
				Layout.topMargin: 5
				Layout.alignment: Qt.AlignTop
				Layout.rightMargin: 5
				Layout.maximumHeight: parent.height * 0.8
				Layout.leftMargin: 5

				onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath) => {
					exerciseSelected(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath);
				}
			}

			RowLayout {
				id: toolbarExercises
				Layout.fillWidth: true
				spacing: 2

				ButtonFlat {
					id:btnNewExercise
					text: qsTr("New")
					enabled: !bEdit

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
						}
						else {
							bNew = false;
							bCanEdit = false;
							exercisesList.enabled = true;
							text = qsTr("New");
							if (!bJustSaved)
								exercisesList.displaySelectedExercise(exercisesList.curIndex);
						}
					}
				} //btnNewExercise

				ButtonFlat {
					id:btnEditExercise
					text: qsTr("Edit")
					enabled: !bNew && exercisesList.curIndex >= 0
					font.capitalization: Font.MixedCase

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
							if (!bJustSaved)
								exercisesList.displaySelectedExercise(exercisesList.curIndex);
						}
					}
				} //btnEditExercise

				ButtonFlat {
					id:btnSaveExercise
					text: qsTr("Save")
					enabled: (bNew && txtExerciseName.length > 5) || (bEdit && txtExerciseName.length > 5)

					onClicked: {
						bJustSaved = true; //Do not issue displaySelectedExercise()
						if (bNew) {
							let results = Database.newExercise(txtExerciseName.text, txtExerciseSubName.text, txtMuscularGroup.text, spinSets.value,
											spinReps.value, spinWeight.value, AppSettings.weightUnit, strMediaPath);
							exercisesList.appendModels(parseInt(results.insertId), txtExerciseName.text, txtExerciseSubName.text,
											txtMuscularGroup.text, spinSets.value, spinReps.value, spinWeight.value,
											AppSettings.weightUnit, strMediaPath);
							btnNewExercise.clicked();
						}
						else if (bEdit) {
							const actualIndex = exercisesList.currentModel.get(exercisesList.curIndex).actualIndex;
							exercisesList.updateModels(actualIndex, txtExerciseName.text, txtExerciseSubName.text, txtMuscularGroup.text,
									spinSets.value, spinReps.value, spinWeight.value, strMediaPath);
							const exerciseId = exercisesList.currentModel.get(exercisesList.curIndex).exerciseId;
							Database.updateExercise(exerciseId, txtExerciseName.text, txtExerciseSubName.text, txtMuscularGroup.text,
									spinSets.value, spinReps.value, spinWeight.value, strMediaPath);
							btnEditExercise.clicked();
						}
						bJustSaved = false;
					}
				} //btnSaveExercise

				ButtonFlat {
					id: btnChooseExercise
					enabled: bChooseButtonEnabled && !bCanEdit && exercisesList.curIndex >= 0
					text: qsTr("Choose")

					onClicked: {
						const curIndex = exercisesList.curIndex;
						exerciseChosen(exercisesList.currentModel.get(curIndex).mainName, exercisesList.currentModel.get(curIndex).subName,
									exercisesList.currentModel.get(curIndex).nSets,	exercisesList.currentModel.get(curIndex).nReps,
									exercisesList.currentModel.get(curIndex).nWeight, true);
						pageExercises.StackView.view.pop();
					}
				} //btnChooseExercise

				ButtonFlat {
					id: btnCancel
					text: qsTr("Close")

					onClicked: pageExercises.StackView.view.pop();
				} // btnCancel

			} // Row
		} //ColumnLayout
	} // footer

	FileDialog {
		id: fileDialog;
		title: qsTr("Please choose a media file");

		onAccepted: {
			strMediaPath = runCmd.getCorrectPath(currentFile);
			close();
			displaySelectedMedia();
		}
	}

	function chooseMediaFromDevice() {
		fileDialog.open();
	}

	function displaySelectedMedia() {
		if (strMediaPath.length < 5)
			return;
		var mediaType = runCmd.getFileType(strMediaPath);
		if ( mediaType === 1) { //video
			if (imageViewer !== null) {
				imageViewer.destroy();
				imageViewer = null;
			}
			if (videoViewer === null) {
				var component = Qt.createComponent("VideoPlayer.qml");
				videoViewer = component.createObject(layoutMain, { mediaSource:strMediaPath} );
			}
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
			if (imageViewer === null) {
				component = Qt.createComponent("ImageViewer.qml");
				imageViewer = component.createObject(layoutMain, { imageSource:strMediaPath });
			}
			else
				imageViewer.imageSource = strMediaPath;
		}
	}
} // Page
