import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../Dialogs"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ColumnLayout {
	id: mainItem
	spacing: 5

	required property Item parentPage
	property int miliseconds
	property bool bMultipleSelection: false
	property bool canDoMultipleSelection: false

	signal exerciseEntrySelected(int idx);
	signal itemDoubleClicked();

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

		function init(idxtoremove): void {
			idxToRemove = idxtoremove;
			start();
		}
	} //Timer

	RowLayout {
		spacing: 5
		height: appSettings.itemDefaultHeight
		Layout.fillWidth: true

		TPLabel {
			text: qsTr("Search: ")
			Layout.preferredWidth: mainItem.width * 0.3
			Layout.leftMargin: 5
		}

		TPRadioButtonOrCheckBox {
			id: chkMultipleSelection
			text: qsTr("Multiple selection")
			radio: false
			visible: canDoMultipleSelection
			Layout.preferredWidth: mainItem.width * 0.6
			Layout.alignment: Qt.AlignRight

			onCheckedChanged: {
				exercisesListModel.clearSelectedEntries();
				bMultipleSelection = checked;
			}
		}
	}

	TPTextInput {
		id: txtSearch
		showClearTextButton: true
		readOnly: !mainItem.enabled
		enabled: exercisesListModel.hasExercises
		Layout.preferredWidth: parent.width * 0.9
		Layout.leftMargin: 5

		onTextChanged: exercisesListModel.search(text);

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

	Rectangle {
		color: "transparent"
		border.color: appSettings.fontColor
		border.width: 2
		radius: 8
		height: parent.height * 0.75
		Layout.fillWidth: true

		TPListView {
			id: lstExercises
			model: exercisesListModel
			anchors.fill: parent
			anchors.margins: 4

			delegate: SwipeDelegate {
				id: delegate
				padding: 5
				width: selected ? lstExercises.width * 1.5 : lstExercises.width
				height: selected ? appSettings.itemExtraLargeHeight * 1.5 : appSettings.itemExtraLargeHeight

				contentItem: TPLabel {
					text: String(index+1) + ":  " + mainName + "\n"+ subName
					leftPadding: 5
					topPadding: 5
					singleLine: false
				}

				background: Rectangle {
					color: selected ? appSettings.entrySelectedColor : "transparent"
					border.color: selected ? appSettings.fontColor : "transparent"
					opacity: selected ? 1 : 0.6
				}

				Behavior on height {
					SpringAnimation {
						easing.type: Easing.InOutQuad
						spring: 3
						damping: 0.2
						mass: 2
					}
				}

				onClicked: itemClicked(index, true);

				swipe.right: Rectangle {
					width: parent.width
					height: parent.height
					color: SwipeDelegate.pressed ? "#555" : "#666"
					radius: 8

					TPImage {
						id: delImage
						source: "remove"
						width: appSettings.itemDefaultHeight
						height: width
						opacity: 2 * -delegate.swipe.position

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
	} //Rectangle

	function itemClicked(idx: int, emit_signal: bool): void {
		if (!bMultipleSelection) {
			if (exercisesListModel.manageSelectedEntries(idx, 1)) {
				exercisesListModel.currentRow = idx;
				if (emit_signal)
					exerciseEntrySelected(idx);
			}
			else {
				itemDoubleClicked();
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
		txtSearch.forceActiveFocus();
	}

	function setFocusToSearchField(): void {
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
				filterDlg = component.createObject(mainwindow, { parentPage: mainItem.parentPage });
				filterDlg.muscularGroupsCreated.connect(function(filterStr) {
					exercisesListModel.setFilter(filterStr);
				});
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		filterDlg.show(btnChooseFilters, 3);
	}
}
