import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "ExercisesAndSets"
import "TPWidgets"
import "Dialogs"
import "Pages"

ApplicationWindow {
	id: mainwindow
	visible: true
	title: "TraininPlanner Tests"

	signal pageActivated_main(Item page);
	signal pageDeActivated_main(Item page);

	//Component.onCompleted: timePicker.show1();

	TPPage {
		id: mainPage
		anchors.fill: parent

		Rectangle {
			anchors.fill: parent
			color: appSettings.paneBackgroundColor

			ColumnLayout {
				anchors {
					fill: parent
					leftMargin: 10
					rightMargin: 10
					topMargin: 10
					bottomMargin: 10
				}

				Row {
					Layout.fillWidth: true
					spacing: 10

					TPLabel {
						text: "Min:"
						width: parent.width * 0.15
					}
					TPTextInput {
						id: randomMin
						width: parent.width * 0.25
						inputMethodHints: Qt.ImhDigitsOnly
						validator: IntValidator { bottom: 0; top: 9999; }
						maximumLength: 4
					}
					TPLabel {
						text: "Max:"
						width: parent.width * 0.1
					}
					TPTextInput {
						id: randomMax
						width: parent.width * 0.2
						inputMethodHints: Qt.ImhDigitsOnly
						validator: IntValidator { bottom: 0; top: 9999; }
						maximumLength: 4
					}
				}

				Row {
					Layout.fillWidth: true
					spacing: 10

					TPButton {
						text: "Generate random number"
						autoSize: true
						flat: false
						enabled: randomMin.text.length > 0 && randomMax.text.length > 0

						onClicked: randomResult.text = String(appUtils.generateRandomNumber(randomMin.text, randomMax.text));
					}
					TPTextInput {
						id: randomResult
						readOnly: true
						width: parent.width * 0.2
					}
				}

				/*TimePicker {
					id: timePicker
					parentPage: mainPage
				}*/

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

				/*CalendarModel {
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
				}*/
			}
		}
	}
}
