import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "ExercisesAndSets"
import "TPWidgets"
import "Dialogs"

ApplicationWindow {
	id: testWindow
	visible: true
	title: "TraininPlanner Tests"

	Rectangle {
		anchors.fill: parent
		color: appSettings.paneBackgroundColor

		ColumnLayout {
			anchors.fill: parent

			/*SetInputField {
				type: SetInputField.TimeType
				availableWidth: testWindow.width * 0.9
				Layout.alignment: Qt.AlignCenter

				onValueChanged: (str) => result.text = str;
			}

			TPPhoneNumberInput {
				id: txtPhone
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: parent.width * 0.85
				Layout.minimumWidth: parent.width * 0.85

				onPhoneNumberOKChanged: result.text = phoneNumberOK ? "Phone OK" : "Phone Not OK"
			}

			TPLabel {
				id: result
				Layout.alignment: Qt.AlignCenter
				Layout.minimumWidth: parent.width * 0.5
			}*/

			CalendarModel {
				id: calModel
				from: new Date(2000, 0, 1)
				to: new Date(2030, 11, 31)

				readonly property bool ready: true //the c++ and qml models must have the same API to avoid warnings and errors
			}

			TPDatePicker {
				calendarModel: calModel
				startDate: calModel.from
				endDate: calModel.to
				selectedDate: new Date()
			}
		}
	}
}
