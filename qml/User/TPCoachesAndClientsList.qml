pragma componentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

Item {
	id: _control

//public:
	property string buttonString: ""
	property bool allowNotConfirmed: false
	property bool listClients: true
	property bool listCoaches: false
	property alias currentRow: listview.currentIndex
	property OnlineUserInfo workingModel: null

	signal itemSelected(userRow: int)
	signal buttonClicked

	TPListView {
		id: listview
		currentIndex: model ? model.currentRow : -1
		height: button.visible ? _control.height - button.height - 5 : _control.height
		model: _control.workingModel
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

			contentItem: TPLabel {
				text: name
				visible: itemVisible
				leftPadding: 5
				bottomPadding: 2
			}

			background: Rectangle {
				color: selected ? AppSettings.entrySelectedColor :
								(index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2)
				opacity: selected ? 1 : 0.8
				border.color: selected ? AppSettings.fontColor : "transparent"

				readonly property bool selected: delegate.index === listview.currentIndex
			}

			onClicked: selectItem(index);
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

	Component.onCompleted: {
		if (listClients && listCoaches)
			workingModel = Qt.binding(function() { return userModel.currentCoachesAndClients; });
		else if (listClients) {
			if (!allowNotConfirmed)
				workingModel = Qt.binding(function() { return userModel.currentClients; });
			else
				workingModel = Qt.binding(function() { return userModel.pendingClientsRequests; });
		}
		else {
			if (!allowNotConfirmed)
				workingModel = Qt.binding(function() { return userModel.currentCoaches; });
			else
				workingModel = Qt.binding(function() { return userModel.pendingCoachesResponses; });
		}
		enabled = Qt.binding(function() { return workingModel ? workingModel.count > 0 : false });
	}

	function selectItem(index: int): void {
		const userrow = workingModel.getUserIdx(index, !(listClients && listCoaches));
		if (userrow > 0) {
			listview.currentIndex = index;
			itemSelected(userrow);
		}
	}

	function applyFilter(filter: string): void {
		workingModel.applyFilter(filter);
	}
}
