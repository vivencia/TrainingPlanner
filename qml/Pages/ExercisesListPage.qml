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

//private:
	property bool _can_edit: false
	property bool _new: false
	property bool _editing: false
	property bool _choose_button_enabled: true
	property TPFileOps videoViewer: null

	signal exerciseChosen();

	onPageActivated: if (AppExercisesList.count > 0)
						 exercisesList.simulateMouseClick(0, true);

	ExercisesListView {
		id: exercisesList
		canDoMultipleSelection: exercisesPage._choose_button_enabled
		height: exercisesPage.height * 0.5

		anchors {
			top: exercisesPage.top
			topMargin: 5
			left: exercisesPage.left
			leftMargin: 5
			right: exercisesPage.right
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
			topMargin: -15
			left: exercisesPage.left
			leftMargin: 5
			right: exercisesPage.right
			rightMargin: 5
			bottom: exercisesPage.bottom
		}

		ColumnLayout {
			id: layoutMain
			anchors.fill: exercisesPage
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
			enabled: exercisesPage._choose_button_enabled && !exercisesPage._can_edit && AppExercisesList.currentRow >= 0
			text: qsTr("Choose")
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: exercisesPage.chooseExercise();
		} //btnChooseExercise

		TPButton {
			id: btnImExport
			text: qsTr("In/Export")
			visible: !exercisesPage._choose_button_enabled
			width: toolbarExercises.buttonWidth
			rounded: false

			onClicked: exercisesPage.showImExMenu();
		} // btnImExport
	} // Row

	function chooseExercise(): void {
		exerciseChosen();
		ItemManager.AppPagesManager.prevPage();
	}

	Loader {
		id: inExMenuLoader
		asynchronous: true
		active: false

		property TPFloatingMenuBar _menu_bar

		sourceComponent: TPFloatingMenuBar {
			parentPage: exercisesPage
			entriesList: [
				{ "label": qsTr("Import"), "image": "import.png", "id": 0, "visible": true },
				{ "label": qsTr("Export"), "image": "save-day.png", "id": 1, "visible": !exercisesPage._choose_button_enabled },
				{ "label": qsTr("Share"), "image": "export.png", "id": 2, "visible": !exercisesPage._choose_button_enabled && Qt.platform.os === "android"},
			]

			onMenuEntrySelected: (id) => {
				switch (id) {
				case 0: exercisesPage.exercisesManager.importExercises(); break;
				case 1: exercisesPage.showExportDlg(false); break;
				case 2: exercisesPage.showExportDlg(true); break;
				}
			}

			onClosed: inExMenuLoader.active = false;
			Component.onCompleted: inExMenuLoader._menu_bar = this;
		}

		onLoaded: _menu_bar.showByWidget(btnImExport, Qt.AlignTop);
	}
	function showImExMenu(): void {
		inExMenuLoader.active = true;
	}

	Loader {
		id: exportDlgLoader
		asynchronous: true
		active: false

		property bool share
		property TPBalloonTip _export_dlg

		sourceComponent: TPBalloonTip {
			message: exportDlgLoader.share ? qsTr("Share custom exercises?") : qsTr("Export custom exercises to file?")
			imageSource:  "export"
			parentPage: exercisesPage
			closeButtonVisible: true
			onButton1Clicked: exercisesPage.exercisesManager.exportExercises(exportDlgLoader.share);
			onClosed: exportDlgLoader.active = false;
			Component.onCompleted: exportDlgLoader._export_dlg = this;
		}

		onLoaded: _export_dlg.showInWindow(-Qt.AlignCenter);
	}
	function showExportDlg(share: bool) : void {
		exportDlgLoader.share = share;
		exportDlgLoader.active = true;
	}
} // Page
