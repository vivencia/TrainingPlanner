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
	property bool bTempDisableChoose: false
	property bool bJustSaved: false
	property var doNotChooseTheseIds: []
	property var imageViewer: null
	property var videoViewer: null

	signal exerciseChosen(string strName1, string strName2, int nSets, real nReps, real nWeight, string uWeight, int exerciseId, bool bAdd)

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
				Layout.leftMargin: 5
				Layout.topMargin: 10
			}
			TextField {
				id: txtExerciseName
				readOnly: !bCanEdit
				font.italic: bCanEdit
				font.pixelSize: AppSettings.fontSizeText
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 20
				font.bold: true

				Keys.onReturnPressed: { //Alphanumeric keyboard
					txtExerciseSubName.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					txtExerciseSubName.forceActiveFocus();
				}
			}

			Label {
				text: qsTr("Specifics:")
				Layout.leftMargin: 5
			}

			TextField {
				id: txtExerciseSubName
				readOnly: !bCanEdit
				font.italic: bCanEdit
				font.bold: true
				font.pixelSize: AppSettings.fontSizeText
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
				Layout.leftMargin: 5
				Layout.topMargin: 10
			}
			TextField {
				id: txtMuscularGroup
				readOnly: !bCanEdit
				font.italic: bCanEdit
				font.pixelSize: AppSettings.fontSizeText
				Layout.fillWidth: true
				Layout.rightMargin: 20
				Layout.leftMargin: 10
				font.bold: true

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
						easing.type: Easing.InOutQuad
					}
				}
				clip: true
				padding: 0
				z: 0

				background: Rectangle {
					border.color: "transparent"
					radius: 5
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
				Layout.bottomMargin: 10
				Layout.topMargin: 10
				Layout.leftMargin: 5
			}

			RowLayout {
				Layout.alignment: Qt.AlignHCenter
				Layout.maximumHeight: 50
				enabled: bCanEdit

				Button {
					id: btnChooseMediaFromDevice
					text: qsTr("File")
					onClicked: chooseMediaFromDevice();

				}
				Button {
					id: btnChooseMediaFromCamera
					text: qsTr("Camera")
					//onClicked: camera.start();
				}
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

		if (bChooseButtonEnabled || bTempDisableChoose) {
			bTempDisableChoose = false;
			for (var i = 0; i < doNotChooseTheseIds.length; ++i) {
				if (exercisesList.mainModel.get(exercisesList.curIndex).exerciseId === doNotChooseTheseIds[i]) {
					bTempDisableChoose = true;
					break;
				}
			}
			bChooseButtonEnabled = !bTempDisableChoose;
		}
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
				spacing: 0

				Button {
					id:btnNewExercise
					text: qsTr("New")
					enabled: !bEdit
					font.capitalization: Font.MixedCase
					display: AbstractButton.TextUnderIcon
					contentItem: Text {
						text: parent.text
						color: "black"
					}

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
								exercisesList.displaySelectedExercise(curIndex);
						}
					}
				} //btnNewExercise

				Button {
					id:btnEditExercise
					text: qsTr("Edit")
					enabled: !bNew && exercisesList.curIndex >= 0
					font.capitalization: Font.MixedCase
					display: AbstractButton.TextUnderIcon
					contentItem: Text {
						text: parent.text
						color: "black"
					}

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
								exercisesList.displaySelectedExercise(curIndex);
						}
					}
				} //btnEditExercise

				Button {
					id:btnSaveExercise
					text: qsTr("Save")
					enabled: (bNew && txtExerciseName.length > 5) || (bEdit && txtExerciseName.length > 5)
					font.capitalization: Font.MixedCase
					display: AbstractButton.TextUnderIcon
					contentItem: Text {
						text: parent.text
						color: "black"
					}

					onClicked: {
						bJustSaved = true; //Do not issue displaySelectedExercise()
						if (bNew) {
							let results = Database.newExercise(txtExerciseName.text, txtExerciseSubName.text, txtMuscularGroup.text, spinSets.value,
											spinReps.value, spinWeight.value, AppSettings.weightUnit, strMediaPath);
							exercisesList.mainModel.append({
								"exerciseId": parseInt(results.insertId),
								"mainName": txtExerciseName.text,
								"subName": txtExerciseSubName.text,
								"muscularGroup": txtMuscularGroup.text,
								"nSets": spinSets.value,
								"nReps": spinReps.value,
								"nWeight": spinWeight.value,
								"uWeight": AppSettings.weightUnit,
								"mediaPath": strMediaPath,
								"actualIndex": exercisesList.mainModel.count
							});
							exercisesList.setCurrentIndex(exercisesList.mainModel.count -1);
							btnNewExercise.clicked();

							if (exercisesList.bFilterApplied) { //There is an active filter. Update the filterModel to reflect the changes
								var regex = new RegExp(txtFilter.text, "i");
								var bFound = false;
								//First look for muscular group
								if (txtMuscularGroup.text.match(regex))
									bFound = true;
								else {
									if (txtExerciseName.text.match(regex))
										bFound = true;
								else
									bFound = false;
								}
								if (bFound)
									exercisesList.tempModel.newItem(curIndex, exercisesList.mainModel.get(exercisesList.curIdx));
							}
						}
						else if (bEdit) {
							const actualIndex = exercisesList.mainModel.get(curIndex).actualIndex;
							exercisesListModel.setProperty(actualIndex, "mainName", txtExerciseName.text);
							exercisesListModel.setProperty(actualIndex, "subName", txtExerciseSubName.text);
							exercisesListModel.setProperty(actualIndex, "muscularGroup", txtMuscularGroup.text);
							exercisesListModel.setProperty(actualIndex, "nSets", spinSets.value);
							exercisesListModel.setProperty(actualIndex, "nReps", spinReps.value);
							exercisesListModel.setProperty(actualIndex, "nWeight", spinWeight.value);
							exercisesListModel.setProperty(actualIndex, "mediaPath", strMediaPath);
							const exerciseId = exercisesListModel.get(actualIndex).exerciseId;
							Database.updateExerciseMainName(exerciseId, txtExerciseName.text);
							Database.updateExerciseSubName(exerciseId, txtExerciseSubName.text);
							Database.updateExerciseMuscularGroup(exerciseId, txtMuscularGroup.text);
							Database.updateExerciseSets(exerciseId, spinSets.value);
							Database.updateExerciseReps(exerciseId, spinReps.value);
							Database.updateExerciseWeight(exerciseId, spinWeight.value);
							Database.updateExerciseMediaPath(exerciseId, strMediaPath);
							btnEditExercise.clicked();

							if (bFilterApplied) { //There is an active filter. The edited item is the current selected item on the list. Just update this item
								exercisesList.tempModel.setProperty(curIndex, "mainName", txtExerciseName.text);
								exercisesList.tempModel.setProperty(curIndex, "subName", txtExerciseSubName.text);
								exercisesList.tempModel.setProperty(curIndex, "muscularGroup", txtMuscularGroup.text);
								exercisesList.tempModel.setProperty(curIndex, "nSets", spinSets.value);
								exercisesList.tempModel.setProperty(curIndex, "nReps", spinReps.value);
								exercisesList.tempModel.setProperty(curIndex, "nWeight", spinWeight.value);
								exercisesList.tempModel.setProperty(curIndex, "mediaPath", strMediaPath);
							}
						}
						bJustSaved = false;
					}
				} //btnSaveExercise

				Button {
					id: btnChooseExercise
					enabled: bChooseButtonEnabled && !bCanEdit && curIndex >= 0
					text: qsTr("Choose")
					font.capitalization: Font.MixedCase
					display: AbstractButton.TextUnderIcon
					contentItem: Text {
						text: parent.text
						color: "black"
					}

					onClicked: {
						exerciseChosen(exercisesList.mainModel.get(curIndex).mainName, exercisesList.mainModel.get(curIndex).subName,
									exercisesList.mainModel.get(curIndex).nSets,	exercisesList.mainModel.get(curIndex).nReps,
									exercisesList.mainModel.get(curIndex).nWeight, exercisesList.mainModel.get(curIndex).uWeight,
									exercisesList.mainModel.get(curIndex).exerciseId, true);
						pageExercises.StackView.view.pop();
					}
				} //btnChooseExercise

				Button {
					id: btnCancel
					text: qsTr("Close")
					font.capitalization: Font.MixedCase
					display: AbstractButton.TextUnderIcon
					contentItem: Text {
						text: parent.text
						color: "black"
					}

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
			console.log("strMediaPath:   ", strMediaPath);
			console.log("currentFile:   ", currentFile);
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
