import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

Item {
	height: optMesoPlan.height + optPreviousDay.height + cboPreviousTDaysDates.height + optLoadFromFile.height + optEmptySession.height

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
		enabled: parentDlg.customBoolProperty1	//bHasMesoPlan

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
		}

		onClicked: parentDlg.customIntProperty1 = 1;
	}

	TPRadioButton {
		id: optPreviousDay
		text: qsTr("Base this session off the one from the one the days in the list below")
		checked: parentDlg.customIntProperty1 === 2
		multiLine: true
		enabled: parentDlg.customBoolProperty2	//bHasPreviousTDays

		anchors {
			left: parent.left
			right: parent.right
			top: optMesoPlan.bottom
		}

		onClicked: parentDlg.customIntProperty1 = 2;
	}

	TPComboBox {
		id: cboPreviousTDaysDates
		model: parentDlg.cboModel
		currentIndex: parentDlg.cboModel.count > 0 ? 0 : -1
		enabled: optPreviousDay.checked
		width: parent.width * 0.7

		anchors {
			horizontalCenter: parent.horizontalCenter
			top: optPreviousDay.bottom
		}

		onActivated: (index) => parentDlg.customStringProperty1 = currentText;
		onEnabledChanged: {
			if (enabled)
				parentDlg.customIntProperty2 = currentValue;
		}
	}

	TPRadioButton {
		id: optLoadFromFile
		text: qsTr("Import workout from file")
		checked: parentDlg.customIntProperty1 === 3
		enabled: parentDlg.customBoolProperty3	//noExercises

		anchors {
			left: parent.left
			right: parent.right
			top: cboPreviousTDaysDates.bottom
		}

		onClicked: parentDlg.customIntProperty1 = 3;
	}

	TPRadioButton {
		id: optEmptySession
		text: qsTr("Start a new session")
		checked: parentDlg.customIntProperty1 === 4

		anchors {
			left: parent.left
			right: parent.right
			top: optLoadFromFile.bottom
		}

		onClicked: parentDlg.customIntProperty1 = 4;
	}
}
