import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "ExercisesAndSets"
import "TPWidgets"
import "Dialogs"

Window {
    id: testWindow
    visible: true
    width: 400
    height: 300
    title: "TraininPlanner Tests"

    ColumnLayout {
        anchors.fill: parent

        SetInputField {
            type: SetInputField.TimeType
            availableWidth: testWindow.width * 0.9
            Layout.alignment: Qt.AlignCenter

            onValueChanged: (str) => result.text = str;
        }

        TPLabel {
            id: result
            Layout.alignment: Qt.AlignCenter
        }
    }
}
