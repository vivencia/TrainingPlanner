import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Column {
	id: grpIntent
	padding: 0
	spacing: 0
	Layout.fillWidth: true

	required property var parentDlg
	required property bool bHasMesoPlan;
	required property bool bHasPreviousTDays;
	required property bool noExercises;

	Component.onCompleted: {
		parentDlg.dialogOpened.connect(resize);
	}

	TPRadioButton {
		id: optMesoPlan
		text: qsTr("Use the standard exercises plan for the division ") + splitLetter + qsTr(" of the Mesocycle")
		visible: bHasMesoPlan
		width: parent.width

		onClicked: parentDlg.customIntProperty1 = 1;
	}

	TPRadioButton {
		id: optPreviousDay
		text: qsTr("Base this session off the one from the one the days in the list below")
		visible: bHasPreviousTDays
		width: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 2;
	}
	TPComboBox {
		id: cboPreviousTDaysDates
		textRole: ""
		model: parentDlg.customModel
		visible: bHasPreviousTDays
		enabled: optPreviousDay.checked
		width: grpIntent.width
		Layout.leftMargin: 20

		onActivated: (index) => {
			parentDlg.customStringProperty1 = currentText;
		}
	}

	TPRadioButton {
		id: optLoadFromFile
		text: qsTr("Import workout from file")
		visible: noExercises
		width: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 3;
	}

	TPRadioButton {
		id: optEmptySession
		text: qsTr("Start a new session")
		width: grpIntent.width

		onClicked: parentDlg.customIntProperty1 = 4;
	}

	function resize() {
		grpIntent.height = optMesoPlan.height + optPreviousDay.height + cboPreviousTDaysDates.height + optLoadFromFile.height + optEmptySession.height;
		if (!bHasMesoPlan)
			grpIntent.height -= optMesoPlan.height;
		if (!bHasPreviousTDays)
			grpIntent.height -= (optPreviousDay.height + cboPreviousTDaysDates.height);
		if (!noExercises)
			grpIntent.height -= optLoadFromFile.height;
	}
}
