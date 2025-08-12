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

	//property list<string> coachesList;

	onOpened: userModel.getOnlineCoachesList();

	TPLabel {
		id: lblTitle
		text: qsTr("Available coaches online")
		horizontalAlignment: Text.AlignHCenter
		height: appSettings.itemDefaultHeight * 1.5

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

		TPLabel {
			text: qsTr("No coaches available")
			font: AppGlobals.largeFont
			horizontalAlignment: Text.AlignHCenter
			visible: userModel.availableCoaches.count === 0
			anchors.fill: parent
		}

		ColumnLayout {
			id: itemsLayout
			anchors.fill: parent

			Repeater {
				id: coachesRepeater
				model: userModel.availableCoaches

				delegate: Row {
					required property string display
					required property bool selected
					required property int index

					Layout.fillWidth: true
					height: 25
					spacing: 0
					padding: 0
					enabled: !userModel.availableCoaches.isUserDefault(index)

					TPRadioButtonOrCheckBox {
						text: display
						radio: false
						multiLine: true
						checked: selected
						width: itemsLayout.width * 0.65

						onClicked: {
							selected = checked;
							userModel.availableCoaches.setSelected(index, selected);
						}
					} //CheckBox

					TPButton {
						text: qsTr("Résumé")
						width: itemsLayout.width*0.3

						onClicked: userModel.viewResume(userModel.availableCoaches, index);
					}
				} //Row
			} //Repeater
		} //ColumnLayout
	} //ScrollView

	TPButton {
		id: btnSendRequest
		text: qsTr("Send request to the selected coaches")
		visible: userModel.availableCoaches ? userModel.availableCoaches.count > 0 : false
		enabled: userModel.availableCoaches ? userModel.availableCoaches.anySelected : false

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
			bottomMargin: 5
		}

		onClicked: userModel.sendRequestToCoaches();
	}
}
