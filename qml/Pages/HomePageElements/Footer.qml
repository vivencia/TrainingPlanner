import QtQuick
import QtQuick.Controls

import "../"
import "../../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPToolBar {
		id: homePageToolBar
		height: 1.5*footerHeight

		TPButton {
			id: btnAddMeso
			text: qsTr("New Training Program")
			imageSource: "mesocycle-add.png"
			backgroundColor: "transparent"
			rounded: false
			flat: true
			width: parent.width

			anchors {
				top: parent.top
				topMargin: 5
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: mesocyclesModel.createNewMesocycle(true);
		}

		TPButton {
			id: btnImportMeso
			text: qsTr("Import program from file")
			imageSource: "import.png"
			backgroundColor: "transparent"
			rounded: false
			flat: true
			width: parent.width

			anchors {
				top: btnAddMeso.bottom
				topMargin: 5
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: itemManager.chooseFileToImport();
		}

		TPButton {
			id: btnWorkout
			text: qsTr("Today's workout")
			imageSource: "workout.png"
			backgroundColor: "transparent"
			rounded: false
			flat: true
			visible: stackView.depth === 1 && mesocyclesModel.canHaveTodaysWorkout
			width: parent.width

			anchors {
				top: btnImportMeso.bottom
				topMargin: 5
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: mesocyclesModel.todaysWorkout();
		}
	}
