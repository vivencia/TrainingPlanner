pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

Item {
	id: _control
	enabled: workingModel.count > 0

//public:
	property string buttonString: ""
	property bool allowNotConfirmed: false
	property bool listClients: true
	property bool listCoaches: false
	property alias currentRow: workingModel.currentRow
	property alias selectedUserIdx: workingModel.currentUserIdx

	signal itemSelected(userIdx: int)
	signal buttonClicked

	UserInfoModel {
		id: workingModel
		showClients: _control.listClients
		showCoaches: _control.listCoaches
		showPending: _control.allowNotConfirmed
	}

	TPListView {
		id: listview
		currentIndex: workingModel.currentRow
		height: button.visible ? _control.height - button.height - 5 : _control.height
		model: workingModel
		enabled: _control.enabled

		anchors {
			top: _control.top
			left: _control.left
			right: _control.right
		}

		delegate: ItemDelegate {
			id: delegate
			spacing: 0
			padding: 5
			width: listview.width
			height: itemVisible ? AppSettings.itemDefaultHeight : 0

			required property int index
			required property string name
			required property bool selected
			required property bool itemVisible

			contentItem: TPLabel {
				text: delegate.name
				visible: delegate.itemVisible
				leftPadding: 5
				bottomPadding: 2
			}

			background: Rectangle {
				color: delegate.selected ? AppSettings.entrySelectedColor :
								(delegate.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2)
				opacity: delegate.selected ? 1 : 0.8
				border.color: delegate.selected ? AppSettings.fontColor : "transparent"

				readonly property bool selected: delegate.index === listview.currentIndex
			}

			onClicked: _control.selectItem(delegate.index);
		} //ItemDelegate
	}

	TPButton {
		id: button
		text: _control.buttonString
		autoSize: true
		rounded: false
		enabled: listview.currentIndex >= 0
		visible: listview.enabled && _control.buttonString.length > 0

		onClicked: _control.buttonClicked();

		anchors {
			bottom: _control.bottom
			horizontalCenter: _control.horizontalCenter
		}
	}

	function selectItem(index: int): void {
		workingModel.currentRow = index;
		itemSelected(workingModel.currentUserIdx);
	}

	function applyFilter(filter: string): void {
		workingModel.applyFilter(filter, AppUserModel.USER_FIELD_NAME);
	}
}
