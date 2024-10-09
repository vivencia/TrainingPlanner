import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Column {
	id: mainItem
	spacing: 5

	property int seconds
	property bool bMultipleSelection: false
	property bool canDoMultipleSelection: false

	signal exerciseEntrySelected(int idx)
	signal itemDoubleClicked()

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
				const newrow = itemManager.removeExercise(idxToRemove)
				simulateMouseClick(newrow, true);
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

	Label {
		Layout.leftMargin: 5
		text: qsTr("Search: ")
		verticalAlignment: Text.AlignVCenter
		font.pointSize: appSettings.fontSizeText
		font.weight: Font.ExtraBold
		color: appSettings.fontColor
		width: parent.width/3
		height: 25

		TPCheckBox {
			id: chkMultipleSelection
			text: qsTr("Multiple selection")
			enabled: canDoMultipleSelection
			height: 25
			width: mainItem.width/2

			anchors {
				left: parent.right
				top: parent.top
			}

			onCheckedChanged: {
				exercisesModel.clearSelectedEntries();
				bMultipleSelection = checked;
			}
		}
	}

	TPTextInput {
		id: txtFilter
		readOnly: !mainItem.enabled
		enabled: exercisesModel.count > 0
		width: parent.width
		Layout.fillWidth: true
		Layout.maximumHeight: 30
		Layout.topMargin: 5
		clip: true

		TPButton {
			id: btnClearText
			imageSource: "edit-clear"
			hasDropShadow: false
			imageSize: 20
			height: 20
			width: 20

			anchors {
				left: txtFilter.right
				leftMargin: -30
				verticalCenter: txtFilter.verticalCenter
			}

			onClicked: {
				txtFilter.clear();
				txtFilter.forceActiveFocus();
			}
		}

		onTextChanged: exercisesModel.setFilter(text, false);
	} // txtFilter

	ListView {
		id: lstExercises
		model: exercisesModel
		boundsBehavior: Flickable.StopAtBounds
		clip: true
		width: parent.width
		height: parent.height * 0.75
		contentHeight: totalHeight * 1.1 + 20//contentHeight: Essencial for the ScrollBars to work.
		contentWidth: totalWidth //contentWidth: Essencial for the ScrollBars to work

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
				color: exercisesModel.currentRow === index ? appSettings.fontColor : "black"
				font.pointSize: appSettings.fontSizeLists
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
				font.pointSize: appSettings.fontSizeLists
			}

			background: Rectangle {
				id:	backgroundColor
				color: selected ? appSettings.entrySelectedColor : index % 2 === 0 ? listEntryColor1 : listEntryColor2
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

				TPImage {
					source: "remove"
					width: 20
					height: 20
					opacity: 2 * -delegate.swipe.position
					z:2

					anchors {
						left: parent.left
						leftMargin: 10
						verticalCenter: parent.verticalCenter
					}
				}

				Label {
					text: qsTr("Removing in " + seconds/1000 + "s")
					color: appSettings.fontColor
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

	function itemClicked(idx: int, emit_signal: bool) {
		if (!bMultipleSelection) {
			if (exercisesModel.manageSelectedEntries(idx, 1)) {
				exercisesModel.currentRow = idx;
				if (emit_signal)
					exerciseEntrySelected(idx);
			}
			else {
				itemDoubleClicked();
				return;
			}
		}
		else {
			if (exercisesModel.manageSelectedEntries(idx, 2)) {
				exercisesModel.currentRow = idx;
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
		txtFilter.text = exercisesModel.getFilter();
	}
}
