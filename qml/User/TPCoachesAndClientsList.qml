import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../TPWidgets"

Item {
	signal itemSelected(userRow: int)
	signal buttonClicked

	property string buttonString: ""
	property bool allowNotConfirmed: false
	property bool listClients: true
	property bool listCoaches: false
	property alias currentIndex: listview.currentIndex
	property OnlineUserInfo workingModel: null

	TPListView {
		id: listview
		currentIndex: model ? model.currentRow : -1
		height: button.visible ? parent.height - button.height - 5 : parent.height
		model: workingModel
		enabled: parent.enabled

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		delegate: ItemDelegate {
			spacing: 0
			padding: 5
			width: listview.width
			height: itemVisible ? appSettings.itemDefaultHeight : 0

			contentItem: TPLabel {
				text: name
				visible: itemVisible
				leftPadding: 5
				bottomPadding: 2
			}

			background: Rectangle {
				readonly property bool selected: index === listview.currentIndex
				color: selected ? appSettings.entrySelectedColor :
								(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
				//opacity: selected ? 1 : 0.8
				border.color: selected ? appSettings.fontColor : "transparent"
			}

			onClicked: selectItem(index);
		} //ItemDelegate
	}

	TPButton {
		id: button
		text: buttonString
		autoSize: true
		rounded: false
		enabled: listview.currentIndex >= 0
		visible: listview.enabled && buttonString.length > 0

		onClicked: buttonClicked();

		anchors {
			bottom: parent.bottom
			horizontalCenter: parent.horizontalCenter
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
