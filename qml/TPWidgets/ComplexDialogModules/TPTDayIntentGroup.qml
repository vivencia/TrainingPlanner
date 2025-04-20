import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

ColumnLayout {
	id: grpIntent
	spacing: 5

	required property TPComplexDialog parentDlg

	Component.onCompleted: parentDlg.customIntProperty1 = 4;

	Connections {
        target: parentDlg
        function onCustomIntProperty1Changed(): void {
			optMesoPlan.checked = parentDlg.customIntProperty1 === 1;
			optPreviousDay.checked = parentDlg.customIntProperty1 === 2;
			optLoadFromFile.checked = parentDlg.customIntProperty1 === 3;
			optEmptySession.checked = parentDlg.customIntProperty1 === 4;
        }
    }

	TPRadioButton {
		id: optMesoPlan
		text: qsTr("Use the standard exercises plan for the division ") + parentDlg.customStringProperty2 + qsTr(" of the Mesocycle")
		checked: parentDlg.customIntProperty1 === 1
		multiLine: true
		visible: parentDlg.customBoolProperty1	//bHasMesoPlan
		Layout.maximumWidth: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 1;
	}

	TPRadioButton {
		id: optPreviousDay
		text: qsTr("Base this session off the one from the one the days in the list below")
		checked: parentDlg.customIntProperty1 === 2
		multiLine: true
		visible: parentDlg.customBoolProperty2	//bHasPreviousTDays
		Layout.maximumWidth: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 2;
	}

	TPComboBox {
		id: cboPreviousTDaysDates
		model: parentDlg.customModel
		visible: parentDlg.customBoolProperty2	//bHasPreviousTDays
		enabled: optPreviousDay.checked
		Layout.alignment: Qt.AlignCenter

		onActivated: (index) => parentDlg.customStringProperty1 = currentText;
		onEnabledChanged: {
			if (enabled)
				parentDlg.customStringProperty1 = currentText;
		}
	}

	TPRadioButton {
		id: optLoadFromFile
		text: qsTr("Import workout from file")
		checked: parentDlg.customIntProperty1 === 3
		visible: parentDlg.customBoolProperty3	//noExercises
		Layout.maximumWidth: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 3;
	}

	TPRadioButton {
		id: optEmptySession
		text: qsTr("Start a new session")
		checked: parentDlg.customIntProperty1 === 4
		Layout.maximumWidth: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 4;
	}
}
