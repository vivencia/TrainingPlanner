import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Column {
	spacing: 0

	property int curIndex: -1
	property int seconds
	property bool bFilterApplied: false
	readonly property ListModel mainModel: lstExercises.model
	readonly property ListModel tempModel: filterModel

	signal exerciseEntrySelected(string exerciseName, string subName, string muscularGroup, int sets, real reps, real weight, string mediaPath)

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
		contentHeight: totalHeight * 1.1 //contentHeight: Essencial for the ScrollBars to work.
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
					//					 : "\ue801" // icon-cancel-circled-1
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
	} // ListView

	Label {
		Layout.leftMargin: 5
		text: qsTr("Filter: ")
		color: "black"
	}

	TextField {
		id: txtFilter
		readOnly: bCanEdit
		enabled: exercisesListModel.count > 0
		width: parent.width
		Layout.fillWidth: true
		Layout.maximumHeight: 40
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

	function displaySelectedExercise(lstIdx) {
		if (lstExercises.count > 0) {
			exerciseEntrySelected(lstExercises.model.get(lstIdx).mainName, lstExercises.model.get(lstIdx).subName,
								lstExercises.model.get(lstIdx).muscularGroup, lstExercises.model.get(lstIdx).nSets,
								lstExercises.model.get(lstIdx).nReps, lstExercises.model.get(lstIdx).nWeight,
			lstExercises.model.get(lstIdx).mediaPath);

			displaySelectedMedia();

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

	function setCurrentIndex(newIdx) {
		if (newIdx < mainModel.count) {
			curIndex = newIdx;
			lstExercises.currentIndex = newIdx;
			lstExercises.ensureVisible(lstExercises.itemAtIndex(newIdx)); //lstExercises.currentItem
		}
	}

	function simulateMouseClick(new_index) {
		displaySelectedExercise(new_index);
		lstExercises.positionViewAtIndex(new_index, ListView.Center);
	}
}
