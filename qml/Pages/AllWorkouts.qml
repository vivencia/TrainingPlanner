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
			focus: true
			startDate: workoutsCalendar.initialDate
			displayDate: new Date()
			endDate: workoutsCalendar.finalDate
			calendarModel: workoutsCalendar
			Layout.alignment: Qt.AlignHCenter

			onDateSelected: (new_date) => workoutsCalendar.selectedDate = new_date;
		}

		TPLabel {
			text: workoutsCalendar.mesoName
			Layout.fillWidth: true
		}
	}
}
