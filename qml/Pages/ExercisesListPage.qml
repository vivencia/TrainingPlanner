pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Exercises

TPPage {
	id: exercisesPage
	imageSource: ":/images/backgrounds/backimage-exercises.jpg"
	backgroundOpacity: 0.6
	objectName: "exercisesPage"

//public:
	required property ExercisesListManager exercisesManager
	property bool chooseButtonEnabled: true

//private:
	property bool _can_edit: false
	property bool _new: false
	property bool _editing: false
	property TPFileOps videoViewer: null

	signal exerciseChosen();

	ExercisesListView {
		id: exercisesList
		canSelectItems: exercisesPage.chooseButtonEnabled
		parentPage: exercisesPage as TPPage
		height: exercisesPage.height * 0.5

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onCurrentIndexChanged: {
			txtExerciseName.text = AppExercisesList.mainName();
			txtExerciseSubName.text = AppExercisesList.subName();
			txtMuscularGroup.text = AppExercisesList.muscularGroup();
		}
	}

	TPScrollView {
		id: scrollExercises
		parentPage: exercisesPage
		navButtonsVisible: enabled
		contentHeight: layoutMain.implicitHeight

		background: Rectangle {
			color: AppSettings.listEntryColor2
			opacity: 0.6
		}

		anchors {
			top: exercisesList.bottom
			left: parent.left
			right: parent.right
			bottom: toolbarExercises.top
		}

		ColumnLayout {
			id: layoutMain
			anchors.fill: parent
			anchors.topMargin: 10

			TPLabel {
				text: AppExercisesList.exerciseNameLabel
			}
			TPTextInput {
				id: txtExerciseName
				readOnly: !exercisesPage._can_edit
				font.italic: exercisesPage._can_edit
				Layout.fillWidth: true
				Layout.rightMargin: 10

				onEnterOrReturnKeyPressed: txtMuscularGroup.forceActiveFocus();
				onEditingFinished: AppExercisesList.setMainName(AppExercisesList.currentRow, text);
			}

			TPLabel {
				text: AppExercisesList.exerciseSpecificsLabel
			}

			TPTextInput {
				id: txtExerciseSubName
				readOnly: !exercisesPage._can_edit
				font.italic: exercisesPage._can_edit
				Layout.fillWidth: true
				Layout.rightMargin: 10

				onEnterOrReturnKeyPressed: txtExerciseSubName.forceActiveFocus();
				onEditingFinished: AppExercisesList.setSubName(AppExercisesList.currentRow, text);
			}

			TPLabel {
				text: AppExercisesList.muscularGroupsLabel
			}
			TPTextInput {
				id: txtMuscularGroup
				readOnly: !exercisesPage._can_edit
				font.italic: exercisesPage._can_edit
				Layout.fillWidth: true
				Layout.rightMargin: 10
				Layout.minimumHeight: 30
				Layout.maximumHeight: 80

				onEditingFinished: AppExercisesList.setMuscularGroup(AppExercisesList.currentRow, text);
			}

			TPLabel {
				text: AppExercisesList.mediaLabel
			}
		} // ColumnLayout
	} // ScrollView

	Row {
		id: toolbarExercises
		spacing: 5
		height: AppSettings.itemDefaultHeight

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			margins: 5
		}
		readonly property int buttonWidth: parent.width / 3 - 10

		TPButton {
			id: btnNewExercise
			text: qsTr("New")
			enabled: !exercisesPage._editing
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: {
				if (!exercisesPage._new) {
					exercisesPage._new = true;
					exercisesPage._can_edit = true;
					scrollExercises.setScrollBarPosition(0);
					txtExerciseName.forceActiveFocus();
					txtExerciseName.clear();
					txtExerciseSubName.clear();
					txtMuscularGroup.clear();
					exercisesList.enabled = false;
					text = qsTr("Cancel");
					AppExercisesList.newExercise();
				} else {
					AppExercisesList.removeRow(AppExercisesList.currentRow);
					exercisesPage._new = false;
					exercisesPage._can_edit = false;
					exercisesList.enabled = true;
					text = qsTr("New");
				}
			}
		} //btnNewExercise

		TPButton {
			id: btnEditExercise
			text: qsTr("Edit")
			enabled: !exercisesPage._new && AppExercisesList.currentRow >= 0
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: {
				if (!exercisesPage._editing) {
					exercisesPage._can_edit = true;
					exercisesPage._editing = true;
					scrollExercises.setScrollBarPosition(0);
					txtExerciseName.forceActiveFocus();
					exercisesList.enabled = false;
					text = qsTr("Cancel");
				} else {
					exercisesPage._can_edit = false;
					exercisesPage._editing = false;
					exercisesList.enabled = true;
					text = qsTr("Edit");
				}
			}
		} //btnEditExercise

		TPButton {
			id: btnChooseExercise
			enabled: exercisesPage.chooseButtonEnabled && !exercisesPage._can_edit && AppExercisesList.currentRow >= 0
			text: qsTr("Choose")
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: exercisesPage.chooseExercise();
		} //btnChooseExercise
	} // Row

	function chooseExercise(): void {
		exerciseChosen();
		ItemManager.appPagesManager.prevPage();
	}
} // Page
