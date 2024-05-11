import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Page {
	id: pageExercises
	width: windowWidth
	height: windowHeight
	title: "Exercises Page"

	property string strMediaPath
	property bool bCanEdit: false
	property bool bNew: false
	property bool bEdit: false
	property bool bChooseButtonEnabled: true
	property bool bJustSaved: false
	property var imageViewer: null
	property var videoViewer: null

	signal exerciseChosen()

	Image { //Avoid painting the same area several times. Use Item as root element rather than Rectangle to avoid painting the background several times.
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

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
			width: pageExercises.width

			Label {
				text: qsTr("Exercise:")
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

				Keys.onReturnPressed: { //Alphanumeric keyboard
					txtExerciseSubName.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					txtExerciseSubName.forceActiveFocus();
				}
			}

			Label {
				text: qsTr("Specifics:")
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

				Keys.onReturnPressed: { //Alphanumeric keyboard
					txtMuscularGroup.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					txtMuscularGroup.forceActiveFocus();
				}
			}

			Label {
				text: qsTr("Muscular Group:")
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

				Keys.onReturnPressed: { //Alphanumeric keyboard
					btnSaveExercise.clicked();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					btnSaveExercise.clicked();
				}
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

				Image {
					anchors.left: lblDefaults.right
					anchors.leftMargin: -20
					anchors.verticalCenter: lblDefaults.verticalCenter
					source: paneExerciseDefaults.shown ? "qrc:/images/"+AppSettings.iconFolder+"fold-up.png" : "qrc:/images/"+AppSettings.iconFolder+"fold-down.png"
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
						alternativeLabels: ["", "", "", qsTr("Sets:")]
						backColor: "transparent"
						borderColor: "transparent"
						labelColor: AppSettings.fontColor
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => text = str;
						onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
					}

					SetInputField {
						id: txtNReps
						type: SetInputField.Type.RepType
						availableWidth: parent.width*0.6
						backColor: "transparent"
						borderColor: "transparent"
						labelColor: AppSettings.fontColor
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => text = str;
						onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
					}

					SetInputField {
						id: txtNWeight
						type: SetInputField.Type.WeightType
						availableWidth: parent.width*0.6
						backColor: "transparent"
						borderColor: "transparent"
						labelColor: AppSettings.fontColor
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => text = str;
						onEnterOrReturnKeyPressed: btnChooseMediaFromDevice.forceActiveFocus();
					}
				} // ColumnLayout
			} //Pane

			Label {
				text: qsTr("Descriptive media:")
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

	footer: ToolBar {
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

		ColumnLayout{
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
					txtExerciseName.text = exercisesListModel.get(index, 1);
					txtExerciseSubName.text = exercisesListModel.get(index, 2);
					txtMuscularGroup.text = exercisesListModel.get(index, 3);
					txtNSets.text = exercisesListModel.get(index, 4);
					txtNReps.text = exercisesListModel.get(index, 5);
					txtNWeight.text = exercisesListModel.get(index, 6);
					strMediaPath = exercisesListModel.get(index, 8);
					displaySelectedMedia();
				}
			}

			RowLayout {
				id: toolbarExercises
				Layout.fillWidth: true
				Layout.maximumHeight: 30
				Layout.leftMargin: 5
				Layout.bottomMargin: 5
				spacing: 5

				readonly property int buttonWidth: Math.ceil(pageExercises.width/5.5)

				TPButton {
					id:btnNewExercise
					text: qsTr("New")
					enabled: !bEdit
					width: toolbarExercises.buttonWidth
					fixedSize: true

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
						}
					}
				} //btnNewExercise

				TPButton {
					id:btnEditExercise
					text: qsTr("Edit")
					enabled: !bNew && exercisesListModel.currentRow >= 0
					width: toolbarExercises.buttonWidth
					fixedSize: true

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
								exercisesList.displaySelectedExercise(exercisesListModel.currentRow);
						}
					}
				} //btnEditExercise

				TPButton {
					id:btnSaveExercise
					text: qsTr("Save")
					enabled: (bNew && txtExerciseName.length > 5) || (bEdit && txtExerciseName.length > 5)
					width: toolbarExercises.buttonWidth
					fixedSize: true

					onClicked: {
						bJustSaved = true; //Do not issue displaySelectedExercise()
						if (bNew) {
							appDB.newExercise(txtExerciseName.text, txtExerciseSubName.text, txtMuscularGroup.text, txtNSets.text,
											txtNReps.text, txtNWeight.text, AppSettings.weightUnit, strMediaPath);
							btnNewExercise.clicked();
							exercisesList.simulateMouseClick(exercisesListModel.count - 1);
						}
						else if (bEdit) {
							exercisesListModel.setCurrentRow(exercisesListModel.currentRow);
							appDB.updateExercise(exercisesListModel.get(exercisesListModel.currentRow, 0), txtExerciseName.text,
													txtExerciseSubName.text, txtMuscularGroup.text, txtNSets.text,
													txtNReps.text, txtNWeight.text, AppSettings.weightUnit, strMediaPath);
							btnEditExercise.clicked();
						}
						bJustSaved = false;
					}
				} //btnSaveExercise

				TPButton {
					id: btnAddExercise
					enabled: bChooseButtonEnabled && !bCanEdit && exercisesListModel.currentRow >= 0
					text: qsTr("Add")
					width: toolbarExercises.buttonWidth
					fixedSize: true

					onClicked: {
						exerciseChosen();
						pageExercises.StackView.view.pop();
					}
				} //btnAddExercise

				TPButton {
					id: btnCancel
					text: qsTr("Close")
					width: toolbarExercises.buttonWidth
					fixedSize: true

					onClicked: pageExercises.StackView.view.pop();
				} // btnCancel

			} // Row
		} //ColumnLayout
	} // footer

	Component.onCompleted: pageExercises.StackView.activating.connect(pageActivation);

	function pageActivation() {
		exercisesList.simulateMouseClick(0, true);
	}

	FileDialog {
		id: fileDialog
		title: qsTr("Please choose a media file");

		onAccepted: {
			strMediaPath = runCmd.getCorrectPath(currentFile);
			close();
			displaySelectedMedia();
		}
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
		var component = Qt.createComponent(obj === 0 ? "ImageViewer.qml" : "VideoPlayer.qml", Qt.Asynchronous);
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
