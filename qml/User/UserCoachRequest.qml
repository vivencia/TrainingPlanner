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

	TPLabel {
		id: lblTitle
		text: qsTr("Available coaches online")
		horizontalAlignment: Text.AlignHCenter
		heightAvailable: 50

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 20
		}
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		contentWidth: availableWidth

		anchors {
			top: lblTitle.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: btnSendRequest.top
			bottomMargin: 5
		}

		ColumnLayout {
			id: itemsLayout
			spacing: 5
			anchors.fill: parent

			Repeater {
				id: coachesRepeater
				model: coachesList

				delegate: Row {
					Layout.fillWidth: true
					height: 25
					spacing: 0
					padding: 5

					TPCheckBox {
						text: coachesList[index]
						width: itemsLayout.width*0.65
						multiLine: true

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
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
			bottomMargin: 5
		}

		onClicked: userModel.sendRequestToCoaches(selectedCoaches);
	}
}
