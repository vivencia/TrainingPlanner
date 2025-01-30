import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

TPPopup {
	id: dlgCoachRequest
	keepAbove: true
	width: appSettings.pageWidth/2
	height: appSettings.pageHeight * 0.5

	property list<string> coachesList;
	Connections {
		target: userModel
		function onCoachesListReceived(coaches_list: list<string>): void {
			coachesList = coaches_list;
		}
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		contentWidth: availableWidth

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			bottom: btnSendRequest.top
			bottomMargin: 5
		}

		ColumnLayout {
			id: itemsLayout
			spacing: 0
			anchors.fill: parent
			anchors.leftMargin: 5

			Repeater {
				id: coachesRepeater
				model: coachesList

				delegate: TPCheckBox {
					text: model[index]
					width: itemsLayout.width
					Layout.fillWidth: true
					height: 25
				}
			} //Repeater
		} //ColumnLayout
	} //ScrollView

	TPButton {
		id: btnSendRequest
		text: qsTr("Send request to the selected coaches")
	}
}
