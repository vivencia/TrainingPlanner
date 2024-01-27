import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Column {
	id: mainItem
	spacing: 5

	property int curIndex: -1
	property int seconds
	property bool bFilterApplied: false
	readonly property ListModel currentModel: lstExercises.model
	readonly property ListModel tempModel: filterModel
	property bool bMultipleSelection: false
	property bool canDoMultipleSelection: false

	//multipleSelectionOption - 0: single selection; 1: remove selection; 2: add selection
	signal exerciseEntrySelected(string exerciseName, string subName, string muscularGroup, int sets,
									real reps, real weight, string mediaPath, int multipleSelectionOption)

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
	} //ListModel

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

	ListView {
		id: lstExercises
		width: parent.width
		height: parent.height * 0.80
		clip: true
		contentHeight: totalHeight * 1.1 + 20//contentHeight: Essencial for the ScrollBars to work.
		contentWidth: totalWidth //contentWidth: Essencial for the ScrollBars to work
		visible: exercisesListModel.count > 0
		boundsBehavior: Flickable.StopAtBounds
		focus: true

		property int totalHeight
		property int totalWidth

		ScrollBar.vertical: ScrollBar {
			id: vBar
			policy: ScrollBar.AsNeeded
			active: true; visible: lstExercises.totalHeight > lstExercises.height
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
				/*if (count > 0) {
					curIndex = 0;
					displaySelectedExercise(curIndex, 0);
				}*/
			}
		} //model

		delegate: SwipeDelegate {
			id: delegate
			property bool bSelected: false
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

			FontMetrics {
				id: fontMetrics
				font.family: listItem.font.family
				font.pixelSize: AppSettings.fontSizeLists
			}

			background: Rectangle {
				id:	backgroundColor
				color: !bMultipleSelection ? curIndex === index ? "cornflowerblue" : index % 2 === 0 ? listEntryColor1 : listEntryColor2 :
							bSelected ? "cornflowerblue" : index % 2 === 0 ? listEntryColor1 : listEntryColor2
			}
			onClicked: {
				if (!bMultipleSelection) {
					if (index !== curIndex) {
						//curIndex = index;
						displaySelectedExercise(index, 0);
					}
					else {
						closeSimpleExerciseList();
						return;
					}
				}
				else {
					bSelected = !bSelected;
					displaySelectedExercise(index, bSelected ? 2 : 1);
				}
				this.forceActiveFocus();
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
					source: "qrc:/images/"+lightIconFolder+"remove.png"
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
	} // ListView

	Label {
		Layout.leftMargin: 5
		text: qsTr("Search: ")
		color: "white"
		font.pixelSize: AppSettings.fontSizeText
		font.bold: true
		width: parent.width - 25

		RoundButton {
			id: btnMultipleSelection
			checkable: true
			checked: false
			visible: canDoMultipleSelection
			width: 20
			height: 20

			Image {
				source: "qrc:/images/" + darkIconFolder + "multi-selection.png"
				width: 20
				height: 20
				anchors.fill: parent
				fillMode: Image.PreserveAspectFit
			}

			anchors {
				left: parent.right
				verticalCenter: parent.verticalCenter
			}

			onCheckedChanged: {
				bMultipleSelection = checked;
			}
		}
	}
	TPTextInput {
		id: txtFilter
		readOnly: !mainItem.enabled
		enabled: exercisesListModel.count > 0
		width: parent.width
		Layout.fillWidth: true
		Layout.maximumHeight: 30
		Layout.topMargin: 5
		clip: true

		ToolButton {
			id: btnClearText
			anchors.left: txtFilter.right
			anchors.leftMargin: -30
			anchors.verticalCenter: txtFilter.verticalCenter
			height: 20
			width: 20
			Image {
				source: "qrc:/images/"+lightIconFolder+"edit-clear.png"
				anchors.fill: parent
				height: 20
				width: 20
			}
			onClicked: {
				txtFilter.clear();
				txtFilter.forceActiveFocus();
			}
		}

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
						filterModel.newItem(i, exercisesListModel.get(i));
					}
				}
				if (!bFilterApplied) {
					bFilterApplied = true;
					lstExercises.setModel(filterModel);
				}
				//simulateMouseClick(0);
			}
			else {
				if (bFilterApplied) {
					bFilterApplied = false;
					lstExercises.setModel(exercisesListModel);
				}
			}
		} //onTextChanged
	} // txtFilter

	function displaySelectedExercise(lstIdx, multiple_opt) {
		curIndex = lstIdx;
		exerciseEntrySelected(currentModel.get(lstIdx).mainName, currentModel.get(lstIdx).subName,
							currentModel.get(lstIdx).muscularGroup, currentModel.get(lstIdx).nSets,
							currentModel.get(lstIdx).nReps, currentModel.get(lstIdx).nWeight,
							currentModel.get(lstIdx).mediaPath, multiple_opt);
	}

	function removeExercise(removeIdx) {
		const actualIndex = currentModel.get(removeIdx).actualIndex; //position of item in the main model
		var i;
		Database.deleteExerciseFromExercises(currentModel.get(actualIndex).exerciseId);
		exercisesListModel.remove(actualIndex);
		if (bFilterApplied) {
			filterModel.remove(removeIdx);
			for (i = removeIdx; i < filterModel.count - 1; ++i ) //Decrease all the actualIndeces for all items after the removed one for the filter model
				filterModel.setProperty(i, "actualIndex", filterModel.get(i).actualIndex - 1);
		}
		for (i = actualIndex; i < exercisesListModel.count - 1; ++i ) //Decrease all the actualIndeces for all items after the removed one
				exercisesListModel.setProperty(i, "actualIndex", i);
		if (curIndex === removeIdx) {
			if (curIndex >= currentModel.count)
				curIndex--;
			if (currentModel.count > 0)
				simulateMouseClick(curIndex);
		}
	}

	function setCurrentIndex(newIdx) {
		if (newIdx < currentModel.count) {
			curIndex = newIdx;
			lstExercises.currentIndex = newIdx;
			lstExercises.ensureVisible(lstExercises.currentItem);
		}
	}

	function simulateMouseClick(new_index) {
		if (new_index < currentModel.count) {
			displaySelectedExercise(new_index, 0);
			lstExercises.positionViewAtIndex(new_index, ListView.Beginning);
		}
	}

	function appendModels(exerciseid, name1, name2, group, nsets, nreps, nweight, media) {
		var actual_idx = exercisesListModel.count;
		exercisesListModel.append ({
			"exerciseId": exerciseid,
			"mainName": name1,
			"subName": name2,
			"muscularGroup": group,
			"nSets": nsets,
			"nReps": nreps,
			"nWeight": nweight,
			"uWeight": AppSettings.weightUnit,
			"mediaPath": media,
			"actualIndex": actual_idx
		});

		if (bFilterApplied) { //There is an active filter. Update the filterModel to reflect the changes
			var regex = new RegExp(txtFilter.text, "i");
			var bFound = false;
			//First look for muscular group
			if (group.text.match(regex))
				bFound = true;
			else {
				if (name1.text.match(regex))
					bFound = true;
				else
					bFound = false;
			}
			if (bFound) {
				filterModel.newItem(actual_idx, exercisesListModel.get(actual_idx));
				setCurrentIndex(filterModel.count - 1); //Make current the last item on the list
			}
		}
		else {
			setCurrentIndex(actual_idx); //Make current the last item on the list
		}

	}

	function updateModels(actual_idx, name1, name2, group, nsets, nreps, nweight, media) {
		exercisesListModel.setProperty(actual_idx, "mainName", name1);
		exercisesListModel.setProperty(actual_idx, "subName", name2);
		exercisesListModel.setProperty(actual_idx, "muscularGroup", group);
		exercisesListModel.setProperty(actual_idx, "nSets", nsets);
		exercisesListModel.setProperty(actual_idx, "nReps", nreps);
		exercisesListModel.setProperty(actual_idx, "nWeight", nweight);
		exercisesListModel.setProperty(actual_idx, "mediaPath", media);
		if (bFilterApplied) { //There is an active filter. The edited item is the current selected item on the list. Just update this item
			filterModel.setProperty(curIndex, "mainName", name1);
			filterModel.setProperty(curIndex, "subName", name2);
			filterModel.setProperty(curIndex, "muscularGroup", group);
			filterModel.setProperty(curIndex, "nSets", nsets);
			filterModel.setProperty(curIndex, "nReps", nreps);
			filterModel.setProperty(curIndex, "nWeight", nweight);
			filterModel.setProperty(curIndex, "mediaPath", media);
		}
	}

	function setFilter(strFilter) {
		txtFilter.text = strFilter;
		txtFilter.textChanged();
	}
}
