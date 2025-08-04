import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

ColumnLayout {
	Layout.fillWidth: true
	required property TPComplexDialog parentDlg

	Component.onCompleted: parentDlg.customIntProperty1 = 4;

	TPButtonGroup {
		id: intentGroup
	}

	TPRadioButtonOrCheckBox {
		id: optMesoPlan
		text: qsTr("Use the standard exercises plan for the division ") + parentDlg.customStringProperty2 + qsTr(" of the Mesocycle")
		checked: parentDlg.customIntProperty1 === 1
		buttonGroup: intentGroup
		multiLine: true
		enabled: parentDlg.customBoolProperty1	//bHasMesoPlan
		Layout.fillWidth: true

		onClicked: parentDlg.customIntProperty1 = 1;
	}

	TPRadioButtonOrCheckBox {
		id: optPreviousDay
		text: qsTr("Base this session off the one from the one the days in the list below")
		checked: parentDlg.customIntProperty1 === 2
		buttonGroup: intentGroup
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
		buttonGroup: intentGroup
		enabled: parentDlg.customBoolProperty3	//noExercises
		Layout.fillWidth: true

		onClicked: parentDlg.customIntProperty1 = 3;
	}

	TPRadioButtonOrCheckBox {
		id: optEmptySession
		text: parentDlg.customStringProperty3
		checked: parentDlg.customIntProperty1 === 4
		buttonGroup: intentGroup
		Layout.fillWidth: true

		onClicked: parentDlg.customIntProperty1 = 4;
	}
}
