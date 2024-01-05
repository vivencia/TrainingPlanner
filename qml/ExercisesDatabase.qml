import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import "jsfunctions.js" as JSF

Page {
	id: pageExercises

	property string strMediaPath
	property int curIndex: -1

	property int seconds
	property bool bCanEdit: false
	property bool bNew: false
	property bool bEdit: false
	property bool bChooseButtonEnabled: false
	property bool bTempDisableChoose: false
	property bool bFilterApplied: false
	property bool bJustSaved: false
	property var doNotChooseTheseIds: []
	property var imageViewer: null
	property var videoViewer: null

	signal exerciseChosen(string strName1, string strName2, int nSets, real nReps, real nWeight, string uWeight, int exerciseId, bool bAdd)

	ListModel {
		id: filterModel
		property var foundIdxs: []

		function newItem(origidx, item) {
			for (var i = 0; i < foundIdxs.length; ++i) {
				if (foundIdxs[i] === origidx)
					return;
			}
			filterModel.append(item);
			foundIdxs.push(origidx);
		}

		function finish() {
			filterModel.clear();
			const len = foundIdxs.length;
			for (var i = 0; i < len; ++i)
				foundIdxs.pop();
		}
	}

	Timer {
		id: undoTimer
		interval: 1000
		property int idxToRemove

		onTriggered: {
			if ( seconds === 0 ) {
				undoTimer.stop();
				removeExercise(idxToRemove);
			}
			else {
				seconds = seconds - 1000;
				start();
			}
		}

		function init(idxtoremove) {
			idxToRemove = idxtoremove;
			start();
		}
	} //Timer

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

			Flickable {
				Layout.fillWidth: true
				Layout.rightMargin: 20
				Layout.leftMargin: 10
				height: contentHeight
				width: parent.width - 20
				contentHeight: txtExerciseSubName.implicitHeight

				TextField {
					id: txtExerciseSubName
					readOnly: !bCanEdit
					font.italic: bCanEdit
					font.bold: true
					font.pixelSize: AppSettings.fontSizeText
					anchors.fill: parent

					Keys.onReturnPressed: { //Alphanumeric keyboard
						txtMuscularGroup.forceActiveFocus();
					}
					Keys.onEnterPressed: { //Numeric keyboard
						txtMuscularGroup.forceActiveFocus();
					}
				}
				ScrollBar.horizontal: ScrollBar { id: hBar2 }
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

	function displaySelectedExercise(lstIdx) {
		txtExerciseName.text = lstExercises.model.get(lstIdx).mainName;
		txtExerciseSubName.text = lstExercises.model.get(lstIdx).subName;
		txtMuscularGroup.text = lstExercises.model.get(lstIdx).muscularGroup;
		spinSets.value = lstExercises.model.get(lstIdx).nSets;
		spinReps.value = lstExercises.model.get(lstIdx).nReps;
		spinWeight.value = lstExercises.model.get(lstIdx).nWeight;
		strMediaPath = lstExercises.model.get(lstIdx).mediaPath;
		displaySelectedMedia();

		hBar2.setPosition(0);

		if (bChooseButtonEnabled || bTempDisableChoose) {
			bTempDisableChoose = false;
			for (var i = 0; i < doNotChooseTheseIds.length; ++i) {
				if (lstExercises.model.get(lstIdx).exerciseId === doNotChooseTheseIds[i]) {
					bTempDisableChoose = true;
					break;
				}
			}
			bChooseButtonEnabled = !bTempDisableChoose;
		}
	}

	function simulateMouseClick(new_index) {
		//if (new_index >= 0 && new_index < lstExercises.model.count) {
			displaySelectedExercise(new_index);
			lstExercises.positionViewAtIndex(new_index, ListView.Center);
		//}
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
			height: bottomPane.height
			spacing: 0

			ListView {
				id: lstExercises
				Layout.fillWidth: true
				Layout.rightMargin: 5
				Layout.minimumHeight: 200
				Layout.maximumHeight: parent.height * 0.5
				Layout.leftMargin: 5
				clip: true
				contentHeight: totalHeight * 1.1 //contentHeight: Essencial for the ScrollBars to work. The 40 is the height of a single itemList
				contentWidth: totalWidth //contentWidth: Essencial for the ScrollBars to work
				visible: exercisesListModel.count > 0
				boundsBehavior: Flickable.StopAtBounds

				property int totalHeight
				property int totalWidth
				property Item item

				ScrollBar.horizontal: ScrollBar {
					id: hBar
					policy: ScrollBar.AsNeeded
					active: true; visible: lstExercises.totalWidth > lstExercises.width
				}
				ScrollBar.vertical: ScrollBar {
					id: vBar
					policy: ScrollBar.AsNeeded
					active: true; visible: lstExercises.totalHeight > lstExercises.height
				}

				function getItem(idx) {
					return listItem.itemAt(idx);
				}

				function ensureVisible(item) {
					var ypos = item.mapToItem(contentItem, 0, 0).y
					var ext = item.height + ypos
					if ( ypos < contentY // begins before
						|| ypos > contentY + height // begins after
						|| ext < contentY // ends before
						|| ext > contentY + height) { // ends after
						// don't exceed bounds
						contentY = Math.max(0, Math.min(ypos - height + item.height, contentHeight - height))
					}
				}

				function setModel(newmodel) {
					model = newmodel;
				}

				model: ListModel {
					id: exercisesListModel

					Component.onCompleted: {
						if (count === 0) {
							let exercises = Database.getExercises();
							for (let exercise of exercises)
								append(exercise);
						}
						if (count > 0) {
							curIndex = 0;
							displaySelectedExercise(curIndex);
						}
					}
				} //model

				FontMetrics {
					id: fontMetrics
					font.family: txtFilter.font.family
					font.pixelSize: AppSettings.fontSizeLists
				}

				delegate: SwipeDelegate {
					id: delegate
					contentItem: Text {
						id: listItem
						text: index+1 + ":  " + mainName + "\n"+ subName
						color: curIndex === index ? "white" : "black"
						font.pixelSize: AppSettings.fontSizeLists
						padding: 0
					}
					spacing: 0
					padding: 0
					width: Math.max(lstExercises.width, fontMetrics.boundingRect(listItem.text).width)
					height: Math.max(40, fontMetrics.boundingRect(listItem.text).height)
					clip: false

					background: Rectangle {
						id:	backgroundColor
						radius: 5
						color: curIndex === index ? "darkred" : index % 2 === 0 ? "#dce3f0" : "#c3cad5"
					}
					onClicked: {
						curIndex = index;
						displaySelectedExercise(index);
					}

					Component.onCompleted: {
						if ( lstExercises.totalWidth < width )
							lstExercises.totalWidth = width;
						lstExercises.totalHeight += height;
					}

					swipe.right: Rectangle {
						width: parent.width
						height: parent.height
						clip: false
						color: SwipeDelegate.pressed ? "#555" : "#666"
						radius: 5

						Image {
							//font.family: "Fontello"
							//text: delegate.swipe.complete ? "\ue805" // icon-cw-circled
							//										 : "\ue801" // icon-cancel-circled-1
							source: "qrc:/images/"+lightIconFolder+"remove.png"
							//anchors.fill: parent
							anchors.left: parent.left
							anchors.leftMargin: 10
							anchors.verticalCenter: parent.verticalCenter
							width: 20
							height: 20
							opacity: 2 * -delegate.swipe.position
							z:2
							//color: Material.color(delegate.swipe.complete ? Material.Green : Material.Red, Material.Shade200)
							//Behavior on color { ColorAnimation { } }
						}

						Label {
							text: qsTr("Removing in " + seconds/1000 + "s")
							color: "white"
							padding: 5
							anchors.fill: parent
							anchors.leftMargin: 40
							horizontalAlignment: Qt.AlignLeft
							verticalAlignment: Qt.AlignVCenter
							opacity: delegate.swipe.complete ? 1 : 0
							Behavior on opacity { NumberAnimation { } }
							z:0
						}

						SwipeDelegate.onClicked: delegate.swipe.close();
						SwipeDelegate.onPressedChanged: undoTimer.stop();
					} //swipe.right

					swipe.onCompleted: {
						seconds = 4000;
						undoTimer.init(index);
					}
				} // SwipeDelegate
			} // Flickable

			Label {
				Layout.leftMargin: 5
				text: qsTr("Filter: ")
				color: "black"
			}
			TextField {
				id: txtFilter
				readOnly: bCanEdit
				enabled: exercisesListModel.count > 0
				Layout.fillWidth: true
				Layout.maximumHeight: 40
				Layout.leftMargin: 5
				Layout.rightMargin: 20
				color: "black"

				onTextChanged: {
					filterModel.finish();
					if (text.length >= 3) {
						var regex = new RegExp(text, "i");
						var bFound = false;
						for(var i = 0; i < exercisesListModel.count; i++ ) {
							//First look for muscular group
							if (exercisesListModel.get(i).muscularGroup.match(regex))
								bFound = true;
							else {
								if (exercisesListModel.get(i).mainName.match(regex))
									bFound = true;
								else
									bFound = false;
							}
							if (bFound) {
								if (!bFilterApplied) {
									lstExercises.setModel(filterModel);
									bFilterApplied = true;
								}
								filterModel.newItem(i, exercisesListModel.get(i));
							}
						}
						if (bFilterApplied)
							simulateMouseClick(0);
					}
					else {
						if (bFilterApplied) {
							bFilterApplied = false;
							lstExercises.setModel(exercisesListModel);
						}
					}
				} //onTextChanged
			} // txtFilter

			RowLayout {
				id: toolbarExercises
				enabled: !undoTimer.running
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
							lstExercises.enabled = false;
							text = qsTr("Cancel");
						}
						else {
							bNew = false;
							bCanEdit = false;
							lstExercises.enabled = true;
							text = qsTr("New");
							if (!bJustSaved)
								displaySelectedExercise(curIndex);
						}
					}
				} //btnNewExercise

				Button {
					id:btnEditExercise
					text: qsTr("Edit")
					enabled: !bNew && curIndex >= 0
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
							hBar2.setPosition(0);
							txtExerciseName.forceActiveFocus();
							lstExercises.enabled = false;
							text = qsTr("Cancel");
						}
						else {
							bCanEdit = false;
							bEdit = false;
							lstExercises.enabled = true;
							text = qsTr("Edit");
							if (!bJustSaved)
								displaySelectedExercise(curIndex);
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
							exercisesListModel.append({
								"exerciseId": parseInt(results.insertId),
								"mainName": txtExerciseName.text,
								"subName": txtExerciseSubName.text,
								"muscularGroup": txtMuscularGroup.text,
								"nSets": spinSets.value,
								"nReps": spinReps.value,
								"nWeight": spinWeight.value,
								"uWeight": AppSettings.weightUnit,
								"mediaPath": strMediaPath,
								"actualIndex": exercisesListModel.count
							});
							curIndex = lstExercises.model.count -1;
							btnNewExercise.clicked();
							lstExercises.currentIndex = curIndex;
							lstExercises.ensureVisible(lstExercises.currentItem);

							if (bFilterApplied) { //There is an active filter. Update the filterModel to reflect the changes
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
									filterModel.newItem(curIndex, exercisesListModel.get(exercisesListModel.count - 1));
							}
						}
						else if (bEdit) {
							const actualIndex = lstExercises.model.get(curIndex).actualIndex;
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
								filterModel.setProperty(curIndex, "mainName", txtExerciseName.text);
								filterModel.setProperty(curIndex, "subName", txtExerciseSubName.text);
								filterModel.setProperty(curIndex, "muscularGroup", txtMuscularGroup.text);
								filterModel.setProperty(curIndex, "nSets", spinSets.value);
								filterModel.setProperty(curIndex, "nReps", spinReps.value);
								filterModel.setProperty(curIndex, "nWeight", spinWeight.value);
								filterModel.setProperty(curIndex, "mediaPath", strMediaPath);
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
						exerciseChosen(lstExercises.model.get(curIndex).mainName, lstExercises.model.get(curIndex).subName,
									lstExercises.model.get(curIndex).nSets,	lstExercises.model.get(curIndex).nReps,
									lstExercises.model.get(curIndex).nWeight, lstExercises.model.get(curIndex).uWeight,
									lstExercises.model.get(curIndex).exerciseId, true);
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
		acceptLabel: (qsTr("Choose"))
		nameFilters: [qsTr("Images (*.jpg *.png *.gif)"), qsTr("Videos (*.mp4)")];
		currentFolder: imagesPath

		onAccepted: {
			strMediaPath = currentFile;
			console.log("strMediaPath:   ", strMediaPath);
			close();
			displaySelectedMedia();
		}
	}

	function removeExercise(removeIdx) {
		const actualIndex = lstExercises.model.get(removeIdx).actualIndex; //position of item in the main model
		var i;
		Database.deleteExerciseFromExercises(lstExercises.model.get(actualIndex).exerciseId);
		exercisesListModel.remove(actualIndex);
		if (bFilterApplied) {
			filterModel.remove(removeIdx);
			for (i = removeIdx; i < filterModel.count - 1; ++i ) //Decrease all the actualIndeces for all items after the removed one for the filter model
				filterModel.setProperty(i, "actualIndex", filterModel.get(i).actualIndex - 1);
		}
		for (i = actualIndex; i < exercisesListModel.count - 1; ++i ) //Decrease all the actualIndeces for all items after the removed one
				exercisesListModel.setProperty(i, "actualIndex", i);
		if (curIndex === removeIdx) {
			if (curIndex >= lstExercises.model.count)
				curIndex--;
			if (lstExercises.model.count > 0)
				simulateMouseClick(curIndex);
		}
	}

	//TODO: improve detection method.
	function mediaType() {
		if (strMediaPath.includes(".mp4"))
			return 1;
		return 0;
	}

	function chooseMediaFromDevice() {
		fileDialog.open();
	}

	function displaySelectedMedia() {
		if (strMediaPath.length < 5)
			return;
		//const mediaPath = JSF.pathToLocalUrl(strMediaPath);
		//console.log("mediaPath:  ", mediaPath);
		if (mediaType() === 0) {
			if (videoViewer !== null) {
				videoViewer.destroy();
				videoViewer = null;
			}
			if (imageViewer === null) {
				console.log("creating image viewer");
				var component = Qt.createComponent("ImageViewer.qml");
				imageViewer = component.createObject(layoutMain, { imageSource:encodeURI(strMediaPath) });
			}
			else
				imageViewer.imageSource = encodeURI(strMediaPath);
		}
		else {
			if (imageViewer !== null) {
				imageViewer.destroy();
				imageViewer = null;
			}
			if (videoViewer === null) {
				component = Qt.createComponent("VideoPlayer.qml");
				videoViewer = component.createObject(layoutMain, { videoSource:encodeURI(strMediaPath) });
			}
			else
				videoViewer.videoSource = encodeURI(strMediaPath);
		}
	}
} // Page
