import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

TPPopup {
	id: dlgCoachRequest
	keepAbove: true
	width: appSettings.pageWidth*0.8
	height: appSettings.pageHeight * 0.4

	required property int userRow
	property list<string> coachesList;
	property list<bool> selectedCoaches;

	Connections {
		target: userModel
		function onCoachesListReceived(coaches_list: list<string>): void {
			coachesList = 0;
			coachesList = coaches_list;
			selectedCoaches.length = 0;
			for (let i = 0; i < coachesList.length; ++i)
				selectedCoaches.push(false);
		}
	}

	onOpened: userModel.getOnlineCoachesList();

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

				delegate: Row {
					Layout.fillWidth: true
					height: 25

					TPCheckBox {
						text: model[index]
						width: itemsLayout.width*0.7

						onCheckedChanged: {
							selectedCoaches[index] = checked;
							for (let i = 0; i < selectedCoaches.length; ++i) {
								if (selectedCoaches[i] === true) {
									btnSendRequest.enabled = true;
									return;
								}
							}
							btnSendRequest.enabled = false;
						}
					} //CheckBox

					TPButton {
						text: qsTr("Résumé")
						width: itemsLayout.width*0.3

						onClicked: userModel.downloadResume(index);
					}
				} //Row
			} //Repeater
		} //ColumnLayout
	} //ScrollView

	TPButton {
		id: btnSendRequest
		text: qsTr("Send request to the selected coaches")
		visible: coachesList.length > 0

		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
			bottomMargin: 5
		}

		onClicked: userModel.sendRequestToCoaches(selectedCoaches);
	}
}
