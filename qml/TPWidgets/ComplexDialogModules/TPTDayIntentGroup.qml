import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

Column {
	id: grpIntent
	padding: 0
	spacing: 5

	required property var parentDlg

	Component.onCompleted: {
		parentDlg.customIntProperty1 = 4;
		parentDlg.bAdjustHeightEveryOpen = true;
		parentDlg.dialogOpened.connect(resize);
	}

	TPRadioButton {
		id: optMesoPlan
		text: qsTr("Use the standard exercises plan for the division ") + parentDlg.customStringProperty2 + qsTr(" of the Mesocycle")
		checked: parentDlg.customIntProperty1 === 1
		multiLine: true
		visible: parentDlg.customBoolProperty1	//bHasMesoPlan
		width: parent.width

		onClicked: parentDlg.customIntProperty1 = 1;
	}

	TPRadioButton {
		id: optPreviousDay
		text: qsTr("Base this session off the one from the one the days in the list below")
		checked: parentDlg.customIntProperty1 === 2
		multiLine: true
		visible: parentDlg.customBoolProperty2	//bHasPreviousTDays
		width: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 2;
	}

	TPComboBox {
		id: cboPreviousTDaysDates
		model: parentDlg.customModel
		visible: parentDlg.customBoolProperty2	//bHasPreviousTDays
		enabled: optPreviousDay.checked
		width: grpIntent.width
		Layout.leftMargin: 20

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
		width: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 3;
	}

	TPRadioButton {
		id: optEmptySession
		text: qsTr("Start a new session")
		checked: parentDlg.customIntProperty1 === 4
		width: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 4;
	}

	function resize(): void {
		grpIntent.height = optMesoPlan.implicitHeight + optPreviousDay.implicitHeight + cboPreviousTDaysDates.height +
								optLoadFromFile.implicitHeight + optEmptySession.implicitHeight;
		if (!parentDlg.customBoolProperty1)
			grpIntent.height -= optMesoPlan.implicitHeight;
		if (!parentDlg.customBoolProperty2)
			grpIntent.height -= (optPreviousDay.implicitHeight + cboPreviousTDaysDates.height);
		if (!parentDlg.customBoolProperty3)
			grpIntent.height -= optLoadFromFile.implicitHeight;
	}
}
