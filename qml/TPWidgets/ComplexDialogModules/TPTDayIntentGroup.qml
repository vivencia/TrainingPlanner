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
		visible: parentDlg.customBoolProperty1	//bHasMesoPlan
		width: parent.width

		onClicked: parentDlg.customIntProperty1 = 1;
	}

	TPRadioButton {
		id: optPreviousDay
		text: qsTr("Base this session off the one from the one the days in the list below")
		visible: parentDlg.customBoolProperty2	//bHasPreviousTDays
		width: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 2;
	}
	TPComboBox {
		id: cboPreviousTDaysDates
		textRole: ""
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
		visible: parentDlg.customBoolProperty3	//noExercises
		width: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 3;
	}

	TPRadioButton {
		id: optEmptySession
		text: qsTr("Start a new session")
		width: grpIntent.width
		checked: true

		onClicked: parentDlg.customIntProperty1 = 4;
	}

	function resize() {
		grpIntent.height = optMesoPlan.height + optPreviousDay.height + cboPreviousTDaysDates.height + optLoadFromFile.height + optEmptySession.height;
		if (!parentDlg.customBoolProperty1)
			grpIntent.height -= optMesoPlan.height;
		if (!parentDlg.customBoolProperty2)
			grpIntent.height -= (optPreviousDay.height + cboPreviousTDaysDates.height);
		if (!parentDlg.customBoolProperty3)
			grpIntent.height -= optLoadFromFile.height;
	}
}
