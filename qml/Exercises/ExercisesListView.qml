pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Dialogs

ColumnLayout {
	id: _control
	spacing: 5

//public:
	property bool bMultipleSelection: false
	property bool canDoMultipleSelection: false

	signal exerciseEntrySelected(int idx);
	signal itemDoubleClicked();

//private:
	property int miliseconds

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
			if (_control.miliseconds === 0) {
				undoTimer.stop();
				const newrow = AppExercisesList.removeExercise(idxToRemove)
				_control.simulateMouseClick(newrow, true);
			}
			else {
				_control.miliseconds -= 1000;
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
		Layout.preferredHeight: AppSettings.itemDefaultHeight
		Layout.fillWidth: true

		TPLabel {
			text: qsTr("Search: ")
			Layout.preferredWidth: _control.width * 0.3
			Layout.leftMargin: 5
		}

		TPRadioButtonOrCheckBox {
			id: chkMultipleSelection
			text: qsTr("Multiple selection")
			radio: false
			visible: _control.canDoMultipleSelection
			Layout.preferredWidth: _control.width * 0.6
			Layout.alignment: Qt.AlignRight

			onCheckedChanged: {
				AppExercisesList.clearSelectedEntries();
				_control.bMultipleSelection = checked;
			}
		}
	}

	TPTextInput {
		id: txtSearch
		showClearTextButton: true
		readOnly: !_control.enabled
		enabled: AppExercisesList.hasExercises
		Layout.preferredWidth: _control.width * 0.9
		Layout.leftMargin: 5

		onTextChanged: AppExercisesList.search(text);

		TPButton {
			id: btnChooseFilters
			imageSource: "filter.png"
			width: AppSettings.itemSmallHeight
			height: width

			anchors {
				left: parent.right
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: _control.showFilterDialog();
		}
	} // txtSearch

	Rectangle {
		color: "transparent"
		border.color: AppSettings.fontColor
		border.width: 2
		radius: 8
		Layout.preferredHeight: _control.height * 0.75
		Layout.fillWidth: true

		TPListView {
			id: lstExercises
			model: AppExercisesList
			anchors.fill: parent
			anchors.margins: 4

			delegate: SwipeDelegate {
				id: delegate
				padding: 5
				width: selected ? lstExercises.width * 1.5 : lstExercises.width
				height: selected ? AppSettings.itemExtraLargeHeight * 1.5 : AppSettings.itemExtraLargeHeight

				required property int index
				required property bool selected
				required property string mainName
				required property string subName

				contentItem: TPLabel {
					text: String(delegate.index+1) + ":  " + delegate.mainName + "\n"+ delegate.subName
					leftPadding: 5
					topPadding: 5
					singleLine: false
				}

				background: Rectangle {
					color: delegate.selected ? AppSettings.entrySelectedColor : "transparent"
					border.color: delegate.selected ? AppSettings.fontColor : "transparent"
					opacity: delegate.selected ? 1 : 0.6
				}

				Behavior on height {
					SpringAnimation {
						easing.type: Easing.InOutQuad
						spring: 3
						damping: 0.2
						mass: 2
					}
				}

				onClicked: _control.itemClicked(delegate.index, true);

				swipe.right: Rectangle {
					width: parent.width
					height: parent.height
					color: SwipeDelegate.pressed ? "#555" : "#666"
					radius: 8

					TPImage {
						id: delImage
						source: "remove"
						width: AppSettings.itemDefaultHeight
						height: width
						opacity: 2 * -delegate.swipe.position

						anchors {
							left: parent.left
							leftMargin: 10
							verticalCenter: parent.verticalCenter
						}
					}

					TPLabel {
						text: qsTr("Removing in ") + parseInt(_control.miliseconds/1000) + "s"
						padding: delImage.width + 20
						anchors.fill: parent
						opacity: delegate.swipe.complete ? 1 : 0
						Behavior on opacity { NumberAnimation {} }
					}

					SwipeDelegate.onClicked: delegate.swipe.close();
					SwipeDelegate.onPressedChanged: undoTimer.stop();
				} //swipe.right

				swipe.onCompleted: {
					_control.miliseconds = 4000;
					undoTimer.init(index);
				}
			} // SwipeDelegate
		} // ListView
	} //Rectangle

	function itemClicked(idx: int, emit_signal: bool): void {
		if (!bMultipleSelection) {
			if (AppExercisesList.manageSelectedEntries(idx, 1)) {
				AppExercisesList.currentRow = idx;
				if (emit_signal)
					exerciseEntrySelected(idx);
			}
			else {
				itemDoubleClicked();
				return;
			}
		}
		else {
			if (AppExercisesList.manageSelectedEntries(idx, 2)) {
				AppExercisesList.currentRow = idx;
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

	Loader {
		id: muscularGroupPickerLoader
		asynchronous: true
		active: false

		property MuscularGroupPicker dialog
		sourceComponent: MuscularGroupPicker {
			onClosed: muscularGroupPickerLoader.active = false;
			Component.onCompleted: muscularGroupPickerLoader.dialog = this;
		}

		onLoaded: dialog.show(btnChooseFilters, Qt.AlignBottom);
	}
	function showFilterDialog(): void {
		muscularGroupPickerLoader.active = true;
	}
}
