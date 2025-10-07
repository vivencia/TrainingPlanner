import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

TPPopup {
	id: dlgCoachRequest
	keepAbove: true
	width: appSettings.pageWidth - 20
	height: appSettings.pageHeight * 0.4

	onOpened: userModel.getOnlineCoachesList();

	TPLabel {
		id: lblTitle
		text: qsTr("Available coaches online")
		height: appSettings.itemDefaultHeight

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: btnClose.width
		}
	}

	ColumnLayout {
		spacing: 5

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
			visible: userModel.availableCoaches ? userModel.availableCoaches.count === 0 : false
			Layout.maximumWidth: parent.width - 10
		}

		ListView {
			id: availableCoachesList
			contentHeight: availableHeight;	contentWidth: availableWidth; clip: true
			spacing: 0
			model: userModel.availableCoaches
			visible: userModel.availableCoaches.count > 0
			Layout.fillWidth: true
			Layout.fillHeight: true

			ScrollBar.vertical: ScrollBar {
				policy: ScrollBar.AsNeeded
				active: true; visible: availableCoachesList.contentHeight > availableCoachesList.height
			}
			ScrollBar.horizontal: ScrollBar {
				policy: ScrollBar.AlwaysOff
				active: false; visible: false
			}

			delegate: Rectangle {
				required property int index
				required property string extraName
				required property bool selected

				height: appSettings.itemDefaultHeight
				width: parent.width
				enabled: !userModel.availableCoaches.isUserDefault(index)
				color: index === availableCoachesList.currentIndex ? appSettings.entrySelectedColor :
					(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)

				TPRadioButtonOrCheckBox {
					id: chkCoachName
					text: extraName
					radio: false
					multiLine: true
					checked: selected
					width: parent.width * 0.65
					x: 5
					y: 0

					onClicked: {
						selected = checked;
						userModel.availableCoaches.setSelected(index, selected);
					}
				} //CheckBox

				TPButton {
					text: qsTr("Résumé")
					rounded: false
					width: parent.width * 0.3
					height: parent.height
					x: parent.width - width - 5
					y: 0
					onClicked: userModel.viewResume(userModel.availableCoaches, index);
				}
			} //delegate
		} //ListView
	} //ColumnLayout

	TPButton {
		id: btnSendRequest
		text: qsTr("Send request to the selected coaches")
		multiline: true
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
