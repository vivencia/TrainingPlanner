pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Widgets

Rectangle {
	id: _control
	border.color: AppSettings.fontColor
	color: "transparent"
	radius: 8

//public:
	property string buttonString: ""
	property string perItemButtonString: ""
	property bool listConfirmed: false
	property bool listAvailable: false
	property bool listClients: false
	property bool listCoaches: false
	property bool multipleSelection: false
	property bool showFilterInput: false
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
			lblEmptyList.visible = count === 0;
			listView.visible = count >= 1;
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
			let text = "";
			if (_control.listClients) {
				if (_control.listConfirmed)
					text = qsTr("No current clients");
				else
					text = qsTr("No clients pending confirmation");
			}
			if (_control.listCoaches) {
				text +=	"<br>";
				if (_control.listConfirmed)
					text +=	qsTr("No current coaches");
				else if (_control.listAvailable)
					text +=	qsTr("No available coaches");
				else
					text +=	qsTr("No coaches pending confirmation");
			}
			return text;
		}
	}

	TPTextInput {
		id: txtFilter
		showClearTextButton: true
		visible: _control.showFilterInput
		onTextChanged: listView.applyFilter(text);

		anchors {
			top: _control.top
			left: _control.left
			right: _control.right
			margins: 2
		}
	}

	TPListView {
		id: listView
		currentIndex: workingModel.currentRow
		height: button.visible ? _control.height - button.height - 5 : _control.height
		model: workingModel
		enabled: _control.enabled
		visible: workingModel.count > 0

		anchors {
			top: _control.showFilterInput ? txtFilter.bottom : _control.top
			left: _control.left
			right: _control.right
			margins: 2
		}

		delegate: TPRadioButtonOrCheckBox {
			id: delegate
			text: name
			boxType: _control.multipleSelection ? TPRadioButtonOrCheckBox.TP_CHECKBOX : TPRadioButtonOrCheckBox.TP_NONE
			image: avatar
			visible: itemVisible
			width: listView.width
			height: itemVisible ? AppSettings.itemDefaultHeight : 0

			required property int index
			required property string name
			required property string avatar
			required property bool selected
			required property bool itemVisible

			background: Rectangle {
				color: delegate.selected ? AppSettings.entrySelectedColor :
								(delegate.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2)
				opacity: delegate.selected ? 1 : 0.8
				border.color: delegate.selected ? AppSettings.fontColor : "transparent"
				radius: 8
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
			bottomMargin: 5
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

	function setSelectedUsers(users: list<string>): void {
		workingModel.setSelectedUsers(users);
	}

	function reset(): void {
		workingModel.applyFilter("");
		workingModel.clearSelection();
	}
}
