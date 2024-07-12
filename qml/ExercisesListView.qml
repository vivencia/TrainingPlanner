import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Column {
	id: mainItem
	spacing: 5

	property int seconds
	property bool bMultipleSelection: false
	property bool canDoMultipleSelection: false

	signal exerciseEntrySelected(int idx)

	//When the list is shared among several objects, if a previous object requested multiple selection and the current
	//does not, bMultipleSelection will be left however the previous caller might have left it. We must make sure
	//the new object does not encounter a list that is doing multiple selection but the button to control it is not visisble
	onCanDoMultipleSelectionChanged: {
		if (!canDoMultipleSelection)
			bMultipleSelection = false;
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

	ListView {
		id: lstExercises
		width: parent.width
		height: parent.height * 0.75
		clip: true
		contentHeight: totalHeight * 1.1 + 20//contentHeight: Essencial for the ScrollBars to work.
		contentWidth: totalWidth //contentWidth: Essencial for the ScrollBars to work
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
			if (item) {
				const ypos = item.mapToItem(contentItem, 0, 0).y;
				const ext = item.height + ypos
				if ( ypos < contentY // begins before
					|| ypos > contentY + height // begins after
					|| ext < contentY // ends before
					|| ext > contentY + height) { // ends after
					// don't exceed bounds
					contentY = Math.max(0, Math.min(ypos - height + item.height, contentHeight - height));
				}
			}
			else
				contentY = 0;
		}

		delegate: SwipeDelegate {
			id: delegate

			contentItem: Text {
				id: listItem
				text: index+1 + ":  " + mainName + "\n"+ subName
				color: exercisesListModel.currentRow === index ? AppSettings.fontColor : "black"
				font.pointSize: AppSettings.fontSizeLists
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
				font.pointSize: AppSettings.fontSizeLists
			}

			background: Rectangle {
				id:	backgroundColor
				color: selected ? AppSettings.entrySelectedColor : index % 2 === 0 ? listEntryColor1 : listEntryColor2
			}

			onClicked: itemClicked(index, true);

			Component.onCompleted: {
				if (lstExercises.totalWidth < width)
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
					source: "qrc:/images/"+AppSettings.iconFolder+"remove.png"
					anchors.left: parent.left
					anchors.leftMargin: 10
					anchors.verticalCenter: parent.verticalCenter
					width: 20
					height: 20
					opacity: 2 * -delegate.swipe.position
					z:2
				}

				Label {
					text: qsTr("Removing in " + seconds/1000 + "s")
					color: AppSettings.fontColor
					padding: 5
					anchors.fill: parent
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
		font.pointSize: AppSettings.fontSizeText
		font.weight: Font.ExtraBold
		color: AppSettings.fontColor
		width: parent.width - 25

		TPCheckBox {
			id: chkMultipleSelection
			text: qsTr("Multiple selection")
			enabled: canDoMultipleSelection
			height: 25

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}

			onCheckedChanged: bMultipleSelection = checked;
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
				source: "qrc:/images/"+AppSettings.iconFolder+"edit-clear.png"
				anchors.fill: parent
				height: 20
				width: 20
			}
			onClicked: {
				txtFilter.clear();
				txtFilter.forceActiveFocus();
			}
		}

		onTextChanged: exercisesListModel.setFilter(text, false);
	} // txtFilter

	Component.onCompleted: {
		function setModel(unique_id) {
			if (unique_id === 2222) {
				appDB.databaseReady.disconnect(setModel);
				lstExercises.model = exercisesListModel;
			}
		}

		appDB.databaseReady.connect(setModel);
		appDB.getAllExercises();
	}

	function removeExercise(removeIdx) {
		const actualIndex = exercisesListModel.getInt(removeIdx, 9); //position of item in the main model
		var i;

		function readyToContinue() {
			appDB.databaseReady.disconnect(readyToContinue);
			if (exercisesListModel.currentRow >= removeIdx)
				simulateMouseClick(exercisesListModel.currentRow);
		}
		exercisesListModel.setCurrentRow(actualIndex);
		appDB.removeExercise(exercisesListModel.getInt(actualIndex, 0));
		appDB.databaseReady.connect(readyToContinue);
	}

	function itemClicked(idx: int, emit_signal: bool) {
		if (!bMultipleSelection) {
			if (exercisesListModel.manageSelectedEntries(idx, 1)) {
				exercisesListModel.currentRow = idx;
				if (emit_signal)
					exerciseEntrySelected(idx);
			}
			else {
				hideSimpleExerciseList();
				return;
			}
		}
		else {
			if (exercisesListModel.manageSelectedEntries(idx, 2)) {
				exercisesListModel.currentRow = idx;
				if (emit_signal)
					exerciseEntrySelected(idx);
			}
		}
		lstExercises.forceActiveFocus();
	}

	function simulateMouseClick(new_index: int, emit_signal: bool) {
		lstExercises.positionViewAtIndex(new_index, ListView.Center);
		itemClicked(new_index, emit_signal);
	}

	function setFilter() {
		txtFilter.text = exercisesListModel.getFilter();
	}
}
