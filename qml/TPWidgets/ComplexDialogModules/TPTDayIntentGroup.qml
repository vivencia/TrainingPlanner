import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

Item {
	id: grpIntent
	height: optMesoPlan.implicitHeight + optPreviousDay.implicitHeight + cboPreviousTDaysDates.height +
				optLoadFromFile.implicitHeight + optEmptySession.implicitHeight + 20

	required property var parentDlg

	Component.onCompleted: {
		parentDlg.customIntProperty1 = 4;
		parentDlg.dialogOpened.connect(resize);
	}

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
		height: visible ? implicitHeight : 0

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		onClicked: parentDlg.customIntProperty1 = 1;
	}

	TPRadioButton {
		id: optPreviousDay
		text: qsTr("Base this session off the one from the one the days in the list below")
		checked: parentDlg.customIntProperty1 === 2
		multiLine: true
		visible: parentDlg.customBoolProperty2	//bHasPreviousTDays
		height: visible ? implicitHeight : 0

		anchors {
			top: parentDlg.customBoolProperty1 ? optMesoPlan.bottom : parent.top
			topMargin: 5
			left: parent.left
			right: parent.right
		}

		onClicked: parentDlg.customIntProperty1 = 2;
	}

	TPComboBox {
		id: cboPreviousTDaysDates
		model: parentDlg.customModel
		visible: parentDlg.customBoolProperty2	//bHasPreviousTDays
		enabled: optPreviousDay.checked
		height: visible ? 25 : 0

		anchors {
			top: optPreviousDay.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 15
			right: parent.right
		}

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
		height: visible ? implicitHeight : 0

		anchors {
			top: parentDlg.customBoolProperty2 ? cboPreviousTDaysDates.bottom : optMesoPlan
			topMargin: 5
			left: parent.left
			right: parent.right
		}

		onClicked: parentDlg.customIntProperty1 = 3;
	}

	TPRadioButton {
		id: optEmptySession
		text: qsTr("Start a new session")
		checked: parentDlg.customIntProperty1 === 4

		anchors {
			top: parentDlg.customBoolProperty3 ? optLoadFromFile.bottom : parentDlg.customBoolProperty2 ?
								cboPreviousTDaysDates.bottom : parentDlg.customBoolProperty1 ? optMesoPlan.bottom : parent.top
			topMargin: 5
			left: parent.left
			right: parent.right
		}

		onClicked: parentDlg.customIntProperty1 = 4;
	}
}
