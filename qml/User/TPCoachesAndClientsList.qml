pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Widgets

Item {
	id: _control

//public:
	property string buttonString: ""
	property string perItemButtonString: ""
	property bool listConfirmed: false
	property bool listAvailable: false
	property bool listClients: false
	property bool listCoaches: false
	property bool multipleSelection: false
	property alias model: workingModel
	property alias anySelected: workingModel.anySelected
	property alias currentRow: workingModel.currentRow
	property alias selectedUserIdx: workingModel.currentUserIdx
	property alias count: workingModel.count

	signal itemSelected(int userIdx)
	signal buttonClicked()
	signal itemButtonClicked(int userIdx)

	UserInfoModel {
		id: workingModel
		showClients: _control.listClients
		showCoaches: _control.listCoaches
		showConfirmed: _control.listConfirmed
		showAvailable: _control.listAvailable
		onCountChanged: {
			if (count === 0)
				lblEmptyList.text = lblEmptyList.updateText();
		}
	}

	TPLabel {
		id: lblEmptyList
		text: updateText();
		horizontalAlignment: Text.AlignHCenter
		visible: workingModel.count === 0

		anchors {
			top: _control.top
			left: _control.left
			right: _control.right
		}

		function updateText(): string {
			if (_control.listClients) {
				if (_control.listConfirmed)
					return qsTr("No current clients");
				else
					return qsTr("No clients pending confirmation");
			}
			else if (_control.listCoaches) {
				if (_control.listConfirmed)
					return qsTr("No current coaches");
				else if (_control.listAvailable)
					return qsTr("No available coaches");
				else
					return qsTr("No coaches pending confirmation");
			}
		}
	}

	TPListView {
		id: listview
		currentIndex: workingModel.currentRow
		height: button.visible ? _control.height - button.height - 5 : _control.height
		model: workingModel
		enabled: _control.enabled
		visible: workingModel.count > 0

		anchors {
			top: _control.top
			left: _control.left
			right: _control.right
		}

		delegate: TPRadioButtonOrCheckBox {
			id: delegate
			text: name
			boxType: _control.multipleSelection ? TPRadioButtonOrCheckBox.TP_CHECKBOX : TPRadioButtonOrCheckBox.TP_NONE
			visible: itemVisible
			width: listview.width
			height: itemVisible ? AppSettings.itemDefaultHeight : 0

			required property int index
			required property string name
			required property bool selected
			required property bool itemVisible

			background: Rectangle {
				color: delegate.selected ? AppSettings.entrySelectedColor :
								(delegate.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2)
				opacity: delegate.selected ? 1 : 0.8
				border.color: delegate.selected ? AppSettings.fontColor : "transparent"
			}

			onClicked: _control.selectItem(delegate.index);

			Loader {
				id: itemButtonLoader
				active: _control.perItemButtonString.length > 0 && delegate.selected
				asynchronous: true
				width: delegate.width * 0.3
				height: AppSettings.itemDefaultHeight * 0.9
				anchors {
					verticalCenter: parent.verticalCenter
					right: parent.right
				}

				sourceComponent: TPButton {
					text: _control.perItemButtonString
					rounded: false
					onClicked: _control.itemButtonClicked(workingModel.userIdx(delegate.index));
				}
			}
		} //ItemDelegate
	}

	TPButton {
		id: button
		text: _control.buttonString
		autoSize: true
		rounded: false
		enabled: workingModel.currentRow >= 0
		visible: _control.buttonString.length > 0

		onClicked: _control.buttonClicked();

		anchors {
			bottom: _control.bottom
			horizontalCenter: _control.horizontalCenter
		}
	}

	function selectItem(index: int): void {
		if (index >= 0 && index < workingModel.count) {
			if (multipleSelection) {
				const is_selected = workingModel.isSelected(index);
				workingModel.setSelected(index, !is_selected);
				workingModel.currentRow = index;
			}
			else {
				workingModel.setSelected(workingModel.currentRow, false);
				workingModel.setSelected(index, true);
				workingModel.currentRow = index;
				itemSelected(workingModel.currentUserIdx);
			}
		}
		else
			itemSelected(-1);
	}

	function applyFilter(filter: string): void {
		workingModel.applyFilter(filter, AppUserModel.USER_FIELD_NAME);
	}

	function selectedUsers(): list<string> {
		return workingModel.selectedUsers();
	}

	function reset(): void {
		workingModel.applyFilter("");
		workingModel.clearSelection();
	}
}
