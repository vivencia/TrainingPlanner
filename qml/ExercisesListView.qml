import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import com.vivenciasoftware.qmlcomponents

Column {
	id: mainItem
	spacing: 5

	property int curIndex: -1
	property int seconds
	property bool bFilterApplied: false

	property bool bMultipleSelection: false
	property bool canDoMultipleSelection: false

	//multipleSelectionOption - 0: single selection; 1: remove selection; 2: add selection
	signal exerciseEntrySelected(string exerciseName, string subName, string muscularGroup, string sets,
									string reps, string weight, string mediaPath, int multipleSelectionOption)

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

		onTextChanged: exercisesListModel.setFilter(text);
	} // txtFilter

	function setModel(model) {
		function readyToProceed() {
			appDB.qmlReady.disconnect(readyToCreate);
			lstExercises.model = model;
		}

		if (model.count === 0)
			loadExercises();
	}

	function displaySelectedExercise(lstIdx, multiple_opt) {
		curIndex = lstIdx;
		exerciseEntrySelected(exercisesListModel.get(lstIdx, 1), exercisesListModel.get(lstIdx, 2),
							exercisesListModel.get(lstIdx, 3), exercisesListModel.get(lstIdx, 4),
							exercisesListModel.get(lstIdx, 5), exercisesListModel.get(lstIdx, 6),
							exercisesListModel.get(lstIdx, 8), multiple_opt);
	}

	function removeExercise(removeIdx) {
		const actualIndex = exercisesListModel.getInt(removeIdx, 9); //position of item in the main model
		var i;

		function readyToContinue() {
			appDB.qmlReady.disconnect();
			if (curIndex === removeIdx) {
				if (curIndex >= exercisesListModel.count)
					curIndex--;
				if (exercisesListModel.count > 0)
					simulateMouseClick(curIndex);
			}
		}
		exercisesListModel.setCurrentRow(actualIndex);
		appDB.pass_object(exercisesListModel);
		appDB.removeExercise(exercisesListModel.getInt(actualIndex, 0));
		appDB.qmlReady.connect(readyToContinue);
	}

	function setCurrentIndex(newIdx) {
		if (newIdx < exercisesListModel.count) {
			curIndex = newIdx;
			lstExercises.currentIndex = newIdx;
			lstExercises.ensureVisible(lstExercises.currentItem);
		}
	}

	function simulateMouseClick(new_index) {
		if (new_index < exercisesListModel.count) {
			displaySelectedExercise(new_index, 0);
			lstExercises.positionViewAtIndex(new_index, ListView.Beginning);
		}
	}

	function setFilter(strFilter) {
		txtFilter.text = strFilter;
		txtFilter.textChanged();
	}
}
