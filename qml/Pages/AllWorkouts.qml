import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: allWorkoutsPage
	objectName: "allWorkoutsPage"

	required property WorkoutsCalendar workoutsCalendar

	ColumnLayout {
		spacing: 5
		anchors {
			fill: parent
			margins: 5
		}

		TPDatePicker {
			id: datePickerControl
			startDate: workoutsCalendar.initialDate
			displayDate: new Date()
			endDate: workoutsCalendar.finalDate
			calendarModel: workoutsCalendar
			sizeFactor: 6.5
			Layout.alignment: Qt.AlignHCenter

			onDateSelected: (new_date) => workoutsCalendar.selectedDate = new_date;
		}

		Rectangle {
			color: "transparent"
			height: 60
			Layout.fillWidth: true
			border.color: workoutsCalendar.workoutCompleted ? "green" : "red"

			RowLayout {
				spacing: 5
				anchors {
					top: parent.top
					left: parent.left
					right: parent.right
				}

				TPLabel {
					text: qsTr("Program:")
					width: parent.width*0.3
				}

				TPLabel {
					text: workoutsCalendar.mesoName
					width: parent.width*0.7
					font.italic: true
				}
			}

			RowLayout {
				spacing: 5
				anchors {
					left: parent.left
					right: parent.right
					bottom: parent.bottom
				}

				TPLabel {
					text: qsTr("Workout #:")
					width: parent.width*0.3
				}

				TPLabel {
					text: workoutsCalendar.trainingDay
					width: parent.width*0.2
					font.italic: true
				}

				TPLabel {
					text: qsTr("Division:")
					width: parent.width*0.3
				}

				TPLabel {
					text: workoutsCalendar.splitLetter
					width: parent.width*0.2
					font.italic: true
				}
			}
		} //Rectangle

		TPButton {
			id: btnViewWorkout
			text: qsTr("Workout")
			imageSource: "workout.png"
			enabled: workoutsCalendar.canViewWorkout
			flat: false
			Layout.alignment: Qt.AlignCenter

			onClicked: workoutsCalendar.viewSelectedWorkout();
		}
	} //ColumnLayout
} //TPPage
