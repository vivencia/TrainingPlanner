import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../Dialogs"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Column {
	id: mainItem
	spacing: 5

	required property Item parentPage
	property int miliseconds
	property bool bMultipleSelection: false
	property bool canDoMultipleSelection: false

	signal exerciseEntrySelected(int idx)
	signal itemDoubleClicked()

	//When the list is shared among several objects, if a previous object requested multiple selection and the current
	//does not, bMultipleSelection will be left however the previous caller might have left it. We must make sure
	//the new object does not encounter a list that is doing multiple selection but the button to control it is not visisble
	onCanDoMultipleSelectionChanged: {
		if (!canDoMultipleSelection) {
			bMultipleSelection = false;
			chkMultipleSelection.checked = false;
		}
	}

	Timer {
		id: undoTimer
		interval: 1000
		property int idxToRemove

		onTriggered: {
			if (miliseconds === 0) {
				undoTimer.stop();
				const newrow = itemManager.removeExercise(idxToRemove)
				simulateMouseClick(newrow, true);
			}
			else {
				miliseconds -= 1000;
				start();
			}
		}

		function init(idxtoremove) {
			idxToRemove = idxtoremove;
			start();
		}
	} //Timer

	Item {
		width: parent.width
		height: 25

		TPLabel {
			text: qsTr("Search: ")
			width: parent.width*0.3
			height: 25

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}
		}

		TPCheckBox {
			id: chkMultipleSelection
			text: qsTr("Multiple selection")
			enabled: canDoMultipleSelection
			width: parent.width*0.6

			anchors {
				right: parent.right
				rightMargin: -50
				verticalCenter: parent.verticalCenter
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
		width: parent.width*0.9
		Layout.topMargin: 5

		onTextChanged: exercisesModel.search(text);

		TPButton {
			id: btnClearText
			imageSource: "edit-clear"
			hasDropShadow: false
			imageSize: 20

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

		TPButton {
			id: btnMuscularGroups
			imageSource: "filter.png"
			imageSize: 25

			anchors {
				left: parent.right
				leftMargin: 15
				verticalCenter: parent.verticalCenter
			}

			onClicked: showFilterDialog();
		}
	} // txtFilter

	ListView {
		id: lstExercises
		model: exercisesModel
		boundsBehavior: Flickable.StopAtBounds
		width: parent.width
		height: parent.height * 0.75
		contentHeight: exercisesModel.count*40*1.1//contentHeight: Essencial for the ScrollBars to work.
		contentWidth: width
		clip: true

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true
			visible: lstExercises.contentHeight > lstExercises.height
			interactive: true
		}

		function ensureVisible(item): void {
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
			spacing: 0
			padding: 0
			width: lstExercises.width
			height: 40

			contentItem: Text {
				id: listItem
				text: index+1 + ":  " + mainName + "\n"+ subName
				color: appSettings.fontColor
				font.pixelSize: appSettings.fontSize
				fontSizeMode: Text.Fit
				leftPadding: 5
				rightPadding: 5
				topPadding: 0
				bottomPadding: 2
			}

			background: Rectangle {
				id:	backgroundColor
				color: selected ? appSettings.entrySelectedColor : index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
			}

			onClicked: itemClicked(index, true);

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
					text: qsTr("Removing in ") + parseInt(miliseconds/1000) + "s"
					color: appSettings.fontColor
					padding: 40
					anchors.fill: parent
					horizontalAlignment: Qt.AlignLeft
					verticalAlignment: Qt.AlignVCenter
					opacity: delegate.swipe.complete ? 1 : 0
					Behavior on opacity { NumberAnimation {} }
					z:0
				}

				SwipeDelegate.onClicked: delegate.swipe.close();
				SwipeDelegate.onPressedChanged: undoTimer.stop();
			} //swipe.right

			swipe.onCompleted: {
				miliseconds = 4000;
				undoTimer.init(index);
			}
		} // SwipeDelegate
	} // ListView

	function itemClicked(idx: int, emit_signal: bool): void {
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

	function simulateMouseClick(new_index: int, emit_signal: bool): void {
		lstExercises.positionViewAtIndex(new_index, ListView.Center);
		itemClicked(new_index, emit_signal);
	}

	property MuscularGroupPicker filterDlg: null
	function showFilterDialog(): void {
		if (filterDlg === null) {
			var component = Qt.createComponent("qrc:/qml/Dialogs/MuscularGroupPicker.qml", Qt.Asynchronous);

			function finishCreation() {
				filterDlg = component.createObject(mainwindow, { parentPage: mainItem.parentPage, groupsSeparator: ", " });
				filterDlg.muscularGroupCreated.connect(function(filterStr) {
									let temp_filter = filterStr;
									do {
										filterStr = temp_filter;
										temp_filter = temp_filter.replace(", ", " ");
									} while (temp_filter !== filterStr);
									exercisesModel.setFilter(filterStr);
				});
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		filterDlg.show(exercisesModel.muscularGroup(exercisesModel.currentRealRow()), btnMuscularGroups, 3);
	}
}
