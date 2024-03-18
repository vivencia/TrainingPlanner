import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
	id: pageDeveloper

	property int optChosen: 0

	ColumnLayout {
		id: colMain
		anchors.fill: parent

		Frame {
			Layout.fillWidth: true

			ColumnLayout {
				anchors.fill: parent

				RadioButton {
					id: optDelMesos
					checked: false
					text: qsTr("Delete MesoCycle Table")

					onCheckedChanged: {
						if (checked) optChosen = 1;
					}
				}
				RadioButton {
					id: optDelMesoCalendar
					checked: false
					text: qsTr("Delete MesoCycle Calendar Table")

					onCheckedChanged: {
						if (checked) optChosen = 2;
					}
				}
				RadioButton {
					id: optDelExercises
					checked: false
					text: qsTr("Delete Exercises Table")

					onCheckedChanged: {
						if (checked) optChosen = 3;
					}
				}
			} //Column Layout
		} //Frame

		ToolButton {
			id: btnExecuteAction
			Layout.alignment: Qt.AlignCenter
			text: qsTr("Run action")
			display: AbstractButton.TextUnderIcon
			font.capitalization: Font.MixedCase
			enabled: optChosen !== 0

			onClicked: {
				switch (optChosen) {
					case 1:
						appDB.deleteMesocyclesTable();
						appDB.deleteMesoSplitTable();
						appDB.deleteMesoCalendarTable();
					break;
					case 2:
						appDB.deleteMesoCalendarTable();
					break;
					case 3:
						appDB.deleteExercisesTable();
					break;
				}
				optChosen = 0;
			}
		}
	} //colMain
}
