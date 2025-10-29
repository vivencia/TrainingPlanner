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
		height: appSettings.itemDefaultHeight

		TPLabel {
			text: qsTr("Search: ")
			width: parent.width * 0.3

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}
		}

		TPRadioButtonOrCheckBox {
			id: chkMultipleSelection
			text: qsTr("Multiple selection")
			radio: false
			enabled: canDoMultipleSelection
			width: parent.width * 0.6

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: 5
			}

			onCheckedChanged: {
				exercisesModel.clearSelectedEntries();
				bMultipleSelection = checked;
			}
		}
	}

	TPTextInput {
		id: txtSearch
		showClearTextButton: true
		readOnly: !mainItem.enabled
		enabled: exercisesModel.hasExercises
		width: parent.width * 0.9
		Layout.topMargin: 5

		onTextChanged: exercisesModel.search(text);

		TPButton {
			id: btnChooseFilters
			imageSource: "filter.png"
			width: appSettings.itemSmallHeight
			height: width

			anchors {
				left: parent.right
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: showFilterDialog();
		}
	} // txtSearch

	ListView {
		id: lstExercises
		model: exercisesModel
		boundsBehavior: Flickable.StopAtBounds
		clip: true
		reuseItems: true
		width: parent.width
		height: parent.height * 0.75
		contentHeight: exercisesModel.count * 40 * 1.1 //contentHeight: Essencial for the ScrollBars to work.
		contentWidth: width

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true
			visible: lstExercises.contentHeight > lstExercises.height
			interactive: true
		}

		/*function ensureVisible(item): void {
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
		}*/

		delegate: SwipeDelegate {
			id: delegate
			padding: 5
			width: lstExercises.width
			height: appSettings.itemExtraLargeHeight

			contentItem: TPLabel {
				id: listItem
				text: index+1 + ":  " + mainName + "\n"+ subName
				leftPadding: 5
				topPadding: 5
			}

			background: Rectangle {
				id:	backgroundColor
				color: selected ? appSettings.entrySelectedColor : index % 2 === 0 ?
													appSettings.listEntryColor1 : appSettings.listEntryColor2
				opacity: 0.9
			}

			onClicked: itemClicked(index, true);

			swipe.right: Rectangle {
				width: parent.width
				height: parent.height
				clip: false
				color: SwipeDelegate.pressed ? "#555" : "#666"
				radius: 5

				TPImage {
					id: delImage
					source: "remove"
					width: appSettings.itemDefaultHeight
					height: width
					opacity: 2 * -delegate.swipe.position
					z: 2

					anchors {
						left: parent.left
						leftMargin: 10
						verticalCenter: parent.verticalCenter
					}
				}

				TPLabel {
					text: qsTr("Removing in ") + parseInt(miliseconds/1000) + "s"
					padding: delImage.width + 20
					anchors.fill: parent
					opacity: delegate.swipe.complete ? 1 : 0
					Behavior on opacity { NumberAnimation {} }
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
		txtSearch.forceActiveFocus();
	}

	function simulateMouseClick(new_index: int, emit_signal: bool): void {
		lstExercises.positionViewAtIndex(new_index, ListView.Center);
		itemClicked(new_index, emit_signal);
	}

	property MuscularGroupPicker filterDlg: null
	function showFilterDialog(): void {
		if (filterDlg === null) {
			let component = Qt.createComponent("qrc:/qml/Dialogs/MuscularGroupPicker.qml", Qt.Asynchronous);

			function finishCreation() {
				filterDlg = component.createObject(mainwindow, { parentPage: mainItem.parentPage, groupsSeparator: '|' });
				filterDlg.muscularGroupCreated.connect(function(filterStr) {
					exercisesModel.setFilter(filterStr);
				});
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		filterDlg.show(exercisesModel.muscularGroup(exercisesModel.currentRealRow()), btnChooseFilters, 3);
	}
}
