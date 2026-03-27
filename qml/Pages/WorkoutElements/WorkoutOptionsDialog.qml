import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
	id: _dialog
	parentPage: parentWorkoutPage
	keepAbove: true
	closeButtonVisible: false
	showTitleBar: true
	focus: true
	width: AppSettings.pageWidth * 0.8
	height: mainLayout.height * 1.1

//public:
	required property DBExercisesModel workoutModel
	required property WorkoutManager workoutManager
	required property TPPage parentWorkoutPage
	required property ListModel prevWorkoutsList
	property int selectedPrevWorkoutDay: -1

	signal selectedOption(int option);

	TPLabel {
		text: _dialog.workoutManager.todaysWorkout ? qsTr("What do you want to do today?") : qsTr("Empty workout options")
		horizontalAlignment: Text.AlignHCenter

		anchors {
			verticalCenter: _dialog.titleBar.verticalCenter
			left: parent.left
			right: parent.right
		}
	}

	ColumnLayout {
		id: mainLayout
		anchors {
			top: _dialog.titleBar.bottom
			left: parent.left
			right: parent.right
			margins: 5
			topMargin: -5
		}

		TPButtonGroup {
			id: intentGroup
		}

		TPRadioButtonOrCheckBox {
			id: optMesoPlan
			text: qsTr("Use the standard exercises plan for the division ") + _dialog.workoutModel.splitLetter + qsTr(" of the Mesocycle")
			buttonGroup: intentGroup
			multiLine: true
			enabled: _dialog.workoutManager.canImportFromSplitPlan
			Layout.fillWidth: true
			Layout.maximumHeight: AppSettings.itemDefaultHeight * 3
		}

		TPRadioButtonOrCheckBox {
			id: optPreviousDay
			text: qsTr("Base this session off a session from a previous day")
			buttonGroup: intentGroup
			multiLine: true
			enabled: _dialog.workoutManager.canImportFromPreviousWorkout
			Layout.fillWidth: true
			Layout.maximumHeight: AppSettings.itemDefaultHeight * 2
		}

		TPComboBox {
			id: cboPreviousTDaysDates
			model: _dialog.prevWorkoutsList
			currentIndex: _dialog.prevWorkoutsList ? 0 : -1
			enabled: optPreviousDay.checked
			Layout.preferredWidth: parent.width * 0.7
			Layout.alignment: Qt.AlignCenter

			onActivated: (index) => _dialog.selectedPrevWorkoutDay = currentValue;
		}

		TPRadioButtonOrCheckBox {
			id: optLoadFromFile
			text: qsTr("Import workout from file")
			buttonGroup: intentGroup
			Layout.fillWidth: true
		}

		TPRadioButtonOrCheckBox {
			id: optEmptySession
			text: _dialog.workoutManager.sessionLabel
			multiLine: true
			buttonGroup: intentGroup
			Layout.fillWidth: true
		}

		TPButton {
			text: qsTr("Proceed")
			autoSize: true
			enabled: intentGroup.selectedOption !== -1
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				_dialog.selectedOption(intentGroup.selectedOption);
				_dialog.closePopup();
			}
		}
	}
}
