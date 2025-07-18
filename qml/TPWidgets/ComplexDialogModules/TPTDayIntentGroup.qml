import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

ColumnLayout {
	Layout.fillWidth: true
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

	TPRadioButtonOrCheckBox {
		id: optMesoPlan
		text: qsTr("Use the standard exercises plan for the division ") + parentDlg.customStringProperty2 + qsTr(" of the Mesocycle")
		checked: parentDlg.customIntProperty1 === 1
		multiLine: true
		enabled: parentDlg.customBoolProperty1	//bHasMesoPlan
		Layout.fillWidth: true

		onClicked: parentDlg.customIntProperty1 = 1;
	}

	TPRadioButtonOrCheckBox {
		id: optPreviousDay
		text: qsTr("Base this session off the one from the one the days in the list below")
		checked: parentDlg.customIntProperty1 === 2
		multiLine: true
		enabled: parentDlg.customBoolProperty2	//bHasPreviousTDays
		Layout.fillWidth: true

		onClicked: parentDlg.customIntProperty1 = 2;
	}

	TPComboBox {
		id: cboPreviousTDaysDates
		model: parentDlg.cboModel
		currentIndex: parentDlg.cboModel.count > 0 ? 0 : -1
		enabled: optPreviousDay.checked
		width: parent.width * 0.7
		Layout.alignment: Qt.AlignCenter

		onActivated: (index) => parentDlg.customStringProperty1 = currentText;
		onEnabledChanged: {
			if (enabled)
				parentDlg.customIntProperty2 = currentValue;
		}
	}

	TPRadioButtonOrCheckBox {
		id: optLoadFromFile
		text: qsTr("Import workout from file")
		checked: parentDlg.customIntProperty1 === 3
		enabled: parentDlg.customBoolProperty3	//noExercises
		Layout.fillWidth: true

		onClicked: parentDlg.customIntProperty1 = 3;
	}

	TPRadioButtonOrCheckBox {
		id: optEmptySession
		text: qsTr("Start a new session")
		checked: parentDlg.customIntProperty1 === 4
		Layout.fillWidth: true

		onClicked: parentDlg.customIntProperty1 = 4;
	}
}
