import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Item {
	signal itemSelected(userRow: int)
	signal buttonClicked

	property string buttonString: ""
	property bool allowNotConfirmed: false
	property bool listClients: true
	property bool listCoaches: false
	property alias currentIndex: listview.currentIndex
	property OnlineUserInfo workingModel: null

	ListView {
		id: listview
		contentHeight: availableHeight
		contentWidth: availableWidth
		spacing: 0
		clip: true
		currentIndex: model ? model.currentRow : -1
		height: button.visible ? 0.8 * parent.height : parent.height
		model: workingModel
		enabled: parent.enabled

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: listview.contentHeight > listview.height
		}
		ScrollBar.horizontal: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: listview.contentWidth > listview.width
		}

		delegate: ItemDelegate {
			spacing: 0
			padding: 5
			width: parent.width
			height: appSettings.itemDefaultHeight

			contentItem: TPLabel {
				text: name
				leftPadding: 5
				bottomPadding: 2
			}

			background: Rectangle {
				color: index === listview.currentIndex ? appSettings.entrySelectedColor :
					(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
				opacity: 0.8
			}

			onClicked: selectItem(index);
		} //ItemDelegate
	}

	RowLayout {
		uniformCellSizes: true
		visible: listview.enabled

		anchors {
			top: listview.bottom
			topMargin: 5
			left: parent.left
			right: parent.right
		}

		TPButton {
			id: button
			text: buttonString
			autoSize: true
			rounded: false
			enabled: listview.currentIndex >= 0
			visible: buttonString.length > 0
			Layout.alignment: Qt.AlignCenter

			onClicked: buttonClicked();
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
		if (!workingModel)
			return;
		const userrow = workingModel.getUserIdx(index, !(listClients && listCoaches));
		if (userrow > 0) {
			workingModel.currentRow = index;
			listview.currentIndex = index;
			itemSelected(userrow);
		}
	}
}
