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

	onPageActivated: if (AppExercisesList.count > 0)
						 exercisesList.simulateMouseClick(0, true);

	ExercisesListView {
		id: exercisesList
		enableMultipleSelection: exercisesPage.chooseButtonEnabled
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

		onExerciseEntrySelected: (index) => {
			txtExerciseName.text = AppExercisesList.mainName(index);
			txtExerciseSubName.text = AppExercisesList.subName(index);
			txtMuscularGroup.text = AppExercisesList.muscularGroup(index);
		}

		onItemDoubleClicked: {
			if (btnChooseExercise.enabled)
				exercisesPage.chooseExercise();
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
			bottom: parent.bottom
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
		spacing: 0
		height: AppSettings.itemDefaultHeight

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
			bottomMargin: 10
		}
		readonly property int buttonWidth: (parent.width - 10) * 0.25

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
				}
				else {
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
				}
				else {
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
		ItemManager.AppPagesManager.prevPage();
	}

	FileOperations {
		id: fileOps
		fileType: AppUtils.FT_TP_EXERCISES
	}

	TPPageMenu {
		id: pageMenu
		parentPage: exercisesPage
		entriesList: [
			{ "label": qsTr("Import"), "image": "download_", "btn_id": TPFileOps.OT_Custom_1, "enabled": enabledCondition(0) },
			{ "label": qsTr("Save as"), "image": "download_", "btn_id": TPFileOps.OT_Download, "enabled": enabledCondition(1) },
			{ "label": qsTr("Send to"), "image": "attach_", "btn_id": TPFileOps.OT_Forward, "enabled": enabledCondition(2) },
			{ "label": qsTr("Share"), "image": "share_", "btn_id": TPFileOps.OT_Share, "enabled": enabledCondition(3) },
		]
		onMenuEntrySelected: (btn_id) => {
			switch (btn_id) {
			case TPFileOps.OT_Custom_1: exercisesPage.exercisesManager.importExercises(); break;
			default: fileOps.doFileOperation(btn_id); break;
			}
		}

		function enabledCondition(menu_entry: int): bool {
			switch (menu_entry) {
			case 0: return true;
			case 1:
			case 2: return !exercisesPage.chooseButtonEnabled;
			case 3: return !exercisesPage.chooseButtonEnabled && Qt.platform.os === "android";
			}
		}

		Connections {
			target: exercisesPage
			function onChooseButtonEnabledChanged(): void {
				for (let i = 1; i <= 3; ++i)
					pageMenu.enableEntry(i, pageMenu.enabledCondition(i));
			}
		}
	}
} // Page
