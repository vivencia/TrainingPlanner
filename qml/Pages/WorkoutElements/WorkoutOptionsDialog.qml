import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: dialog
	parentPage: parentWorkoutPage
	keepAbove: true
	closeButtonVisible: false
	showTitleBar: true
	focus: true
	width: appSettings.pageWidth * 0.8
	height: mainLayout.height * 1.1

	required property DBExercisesModel workoutModel
	required property WorkoutManager workoutManager
	required property Page parentWorkoutPage
	property ListModel prevWorkoutsList
	property string selectedPrevWorkout

	signal selectedOption(int option);

	TPLabel {
		text: workoutManager.todaysWorkout ? qsTr("What do you want to do today?") : qsTr("Empty workout options")
		horizontalAlignment: Text.AlignHCenter

		anchors {
			verticalCenter: titleBar.verticalCenter
			left: parent.left
			right: parent.right
		}
	}

	ColumnLayout {
		id: mainLayout
		anchors {
			top: titleBar.bottom
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
			text: qsTr("Use the standard exercises plan for the division ") + workoutModel.splitLetter + qsTr(" of the Mesocycle")
			buttonGroup: intentGroup
			multiLine: true
			enabled: workoutManager.canImportFromSplitPlan
			Layout.fillWidth: true
			Layout.maximumHeight: appSettings.itemDefaultHeight * 3
		}

		TPRadioButtonOrCheckBox {
			id: optPreviousDay
			text: qsTr("Base this session off the one from the one the days in the list below")
			buttonGroup: intentGroup
			multiLine: true
			enabled: workoutManager.canImportFromPreviousWorkout
			Layout.fillWidth: true
			Layout.maximumHeight: appSettings.itemDefaultHeight * 2
		}

		TPComboBox {
			id: cboPreviousTDaysDates
			model: prevWorkoutsList
			currentIndex: prevWorkoutsList ? 0 : -1
			enabled: optPreviousDay.checked
			width: parent.width * 0.7
			Layout.alignment: Qt.AlignCenter

			onActivated: (index) => selectedPrevWorkout = currentText;
		}

		TPRadioButtonOrCheckBox {
			id: optLoadFromFile
			text: qsTr("Import workout from file")
			buttonGroup: intentGroup
			Layout.fillWidth: true
		}

		TPRadioButtonOrCheckBox {
			id: optEmptySession
			text: workoutManager.sessionLabel
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
				selectedOption(intentGroup.selectedOption);
				dialog.closePopup();
			}
		}
	}
}
