pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
	id: dlgCoachRequest
	keepAbove: true
	width: AppSettings.pageWidth - 20
	height: AppSettings.pageHeight * 0.4

	onOpened: AppUserModel.getOnlineCoachesList();

	TPLabel {
		id: lblTitle
		text: qsTr("Available coaches online")
		height: AppSettings.itemDefaultHeight

		anchors {
			top: dlgCoachRequest.contentItem.top
			topMargin: 5
			left: dlgCoachRequest.contentItem.left
			leftMargin: 5
			right: dlgCoachRequest.contentItem.right
			rightMargin: dlgCoachRequest.btnClose.width
		}
	}

	ColumnLayout {
		spacing: 5

		anchors {
			top: lblTitle.bottom
			topMargin: 5
			left: dlgCoachRequest.contentItem.left
			leftMargin: 5
			right: dlgCoachRequest.contentItem.right
			rightMargin: 5
			bottom: btnSendRequest.top
			bottomMargin: 5
		}

		TPLabel {
			text: qsTr("No coaches available")
			font: AppGlobals.largeFont
			horizontalAlignment: Text.AlignHCenter
			visible: AppUserModel.availableCoaches ? AppUserModel.availableCoaches.count === 0 : false
			Layout.maximumWidth: parent.width - 10
		}

		TPListView {
			id: availableCoachesList
			model: AppUserModel.availableCoaches
			visible: AppUserModel.availableCoaches.count > 0
			Layout.fillWidth: true
			Layout.fillHeight: true

			delegate: Rectangle {
				id: delegate
				height: AppSettings.itemDefaultHeight
				width: parent.width
				enabled: !AppUserModel.availableCoaches.isUserDefault(index)
				color: index === availableCoachesList.currentIndex ? AppSettings.entrySelectedColor :
					(index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2)

				required property int index
				required property string extraName
				required property bool selected

				TPRadioButtonOrCheckBox {
					id: chkCoachName
					text: delegate.extraName
					radio: false
					multiLine: true
					checked: delegate.selected
					width: parent.width * 0.65
					x: 5
					y: 0

					onClicked: {
						delegate.selected = checked;
						AppUserModel.availableCoaches.setSelected(delegate.index, delegate.selected);
					}
				} //CheckBox

				TPButton {
					text: qsTr("Résumé")
					rounded: false
					width: delegate.width * 0.3
					height: delegate.height
					x: delegate.width - width - 5
					y: 0
					onClicked: AppUserModel.viewResume(AppUserModel.availableCoaches, delegate.index);
				}
			} //delegate
		} //ListView
	} //ColumnLayout

	TPButton {
		id: btnSendRequest
		text: qsTr("Send request to the selected coaches")
		multiline: true
		visible: AppUserModel.availableCoaches ? AppUserModel.availableCoaches.count > 0 : false
		enabled: AppUserModel.availableCoaches ? AppUserModel.availableCoaches.anySelected : false

		anchors {
			left: dlgCoachRequest.contentItem.left
			leftMargin: 5
			right: dlgCoachRequest.contentItem.right
			rightMargin: 5
			bottom: dlgCoachRequest.contentItem.bottom
			bottomMargin: 5
		}

		onClicked: AppUserModel.sendRequestToCoaches();
	}
}
