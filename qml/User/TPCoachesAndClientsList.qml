pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Pages
import TpQml.Widgets

Rectangle {
	id: _control
	border.color: AppSettings.fontColor
	color: "transparent"
	radius: 8

//public:
	required property TPPage parentPage
	property string buttonString: ""
	property string perItemButtonString: ""
	property bool listConfirmed: false
	property bool listAvailable: false
	property bool listClients: false
	property bool listCoaches: false
	property bool multipleSelection: false
	property bool showFilterInput: false
	property alias model: _control.workingModel
	property alias anySelected: _control.workingModel.anySelected
	property alias selectedUsers: _control.workingModel.selectedUsers
	property alias currentRow: _control.workingModel.currentRow
	property alias count: _control.workingModel.count
	property alias identifier: _control.workingModel.identifier
	property alias showHeader: listView.showHeader
	property alias canSearch: listView.canSearch
	property alias canSelectItems: listView.showHeader
	property alias canSort: listView.canSort

	property UserInfoModel workingModel: UserInfoModel {
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

	signal itemSelected(int userIdx)
	signal buttonClicked()
	signal itemButtonClicked(int userIdx)

	TPLabel {
		id: lblEmptyList
		text: updateText();
		horizontalAlignment: Text.AlignHCenter
		visible: _control.workingModel.count === 0

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

	TPListView {
		id: listView
		listModel: _control.workingModel
		parentPage: _control.parentPage
		canFilter: false
		height: button.visible ? _control.height - button.height - 5 : _control.height
		enabled: _control.enabled
		visible: _control.workingModel.count > 0

		anchors {
			top: _control.top
			left: _control.left
			right: _control.right
			margins: 2
		}

		delegate: TPListViewDelegate {
			id: delegate
			_index: index
			_itemVisible: itemVisible
			tpListView: listView

			required property int index
			required property bool itemVisible
			required property bool selected

			required property string name
			required property string avatar

			itemContent: TPRadioButtonOrCheckBox {
				text: delegate.name
				boxType: _control.multipleSelection ? TPRadioButtonOrCheckBox.TP_CHECKBOX : TPRadioButtonOrCheckBox.TP_NONEBOX
				image: delegate.avatar
				enableCheckOutsideIndicator: false
				isChecked: delegate.selected
				backColor: delegate.selected ? AppSettings.entrySelectedColor :
								(delegate.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2)
				visible: delegate.itemVisible
				width: listView.width
				height: delegate.itemVisible ? preferredHeight : 0

				onChecked: (check) => {
					_control.workingModel.setIsSelected(delegate.index, check);
					if (check)
						_control.itemSelected(delegate.index);
				}

				Loader {
					id: itemButtonLoader
					active: _control.perItemButtonString.length > 0 && delegate.index === _control.workingModel.currentRow
					asynchronous: true
					width: delegate.width * 0.3
					height: AppSettings.itemDefaultHeight * 0.9
					anchors {
						verticalCenter: parent.verticalCenter
						right: parent.right
					}

					sourceComponent: TPButton2 {
						text: _control.perItemButtonString
						rounded: false
						onClicked: _control.itemButtonClicked(_control.workingModel.realRow(delegate.index));
					}
				}
			} //itemContent
		} //delegate
	}

	TPButton2 {
		id: button
		text: _control.buttonString
		rounded: false
		enabled: _control.workingModel.currentRow >= 0
		visible: _control.buttonString.length > 0

		onClicked: _control.buttonClicked();

		anchors {
			bottom: _control.bottom
			bottomMargin: 5
			horizontalCenter: _control.horizontalCenter
		}
	}
}
