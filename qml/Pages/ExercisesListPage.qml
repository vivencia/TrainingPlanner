pragma componentBehaviour: Bound

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
		parentPage: exercisesPage
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

			onClicked: exercisesPage.showInExMenu();
		} // btnImExport

	} // Row

	function chooseExercise(): void {
		exerciseChosen();
		ItemManager.appPagesManager.prevPage();
	}

	property TPFloatingMenuBar imExportMenu: null
	readonly property bool bExportEnabled: !exercisesPage._choose_button_enabled
	onBExportEnabledChanged: {
		if (imExportMenu) {
			imExportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				imExportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	Loader {
		id: inExMenuLoader
		asynchronous: true
		active: false

		sourceComponent: TPFloatingMenuBar {
			parentPage: exercisesPage
			entriesList: [ QtObject {
					property string label: qsTr("Import");
					property string image: "import.png";
					property int id: 0;
					property bool visible: true},

				QtObject {
					property string label: qsTr("Export");
					property string image: "save-day.png";
					property int id: 1;
					property bool visible: !exercisesPage._choose_button_enabled},

				QtObject {
					property string label: qsTr("Share");
					property string image: "export.png";
					property int id: 2;
					property bool visible: !exercisesPage._choose_button_enabled && Qt.platform.os === "android"} ]

			onMenuEntrySelected: (id) => {
				switch (id) {
				case 0: exercisesPage.exercisesManager.importExercises(); break;
				case 1: exercisesPage.showExportDlg(false); break;
				case 2: exercisesPage.showExportDlg(true); break;
				}
			}

			onClosed: inExMenuLoader.active = false;
		}

		onLoaded: item.show2(btnImExport, 0);
	}

	Loader {
		id: exportDlgLoader
		asynchronous: true
		active: false

		property bool share

		sourceComponent: TPBalloonTip {
			message: exportDlgLoader.share ? qsTr("Share custom exercises?") : qsTr("Export custom exercises to file?")
			imageSource:  "export"
			parentPage: exercisesPage
			closeButtonVisible: true
			onButton1Clicked: exercisesPage.exercisesManager.exportExercises(exportDlgLoader.share);
			onClosed: exportDlgLoader.active = false;
		}

		onLoaded: item.show(-1);
	}
	function showExportDlg(share: bool) : void {
		exportDlgLoader.share = share;
		exportDlgLoader.active = true;
	}
} // Page
