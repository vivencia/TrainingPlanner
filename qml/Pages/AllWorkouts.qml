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

	TPDatePicker {
		id: datePickerControl
		focus: true
		startDate: workoutsCalendar.initialDate
		displayDate: new Date()
		endDate: workoutsCalendar.finalDate
		calendarModel: workoutsCalendar
	}
}
