import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Item {
	signal clientSelected(userRow: int)
	signal buttonClicked

	property string buttonString: ""
	property int clientRow: -1
	property bool allowNotConfirmedClients: true
	property alias currentIndex: listview.currentIndex

	ListView {
		id: listview
		contentHeight: availableHeight
		contentWidth: availableWidth
		spacing: 0
		clip: true
		model: userModel.clientsNames
		enabled: userModel.haveClients
		currentIndex: clientRow
		height: button.visible ? 0.8 * parent.height : parent.height

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
			height: userSettings.itemDefaultHeight

			contentItem: Text {
				text: modelData
				color: userSettings.fontColor
				font.pixelSize: userSettings.fontSize
				fontSizeMode: Text.Fit
				leftPadding: 5
				bottomPadding: 2
			}

			background: Rectangle {
				color: index === listview.currentIndex ? userSettings.entrySelectedColor :
					(index % 2 === 0 ? userSettings.listEntryColor1 : userSettings.listEntryColor2)
			}

			onClicked: {
				if (!allowNotConfirmedClients) {
					if (userModel.clientsNames[index].indexOf('!') >= 0)
						return;
				}
				selectItem(index);
			}
		} //ItemDelegate
	}

	RowLayout {
		uniformCellSizes: true
		visible: userModel.haveClients

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
			enabled: userModel.currentRow !== 0
			visible: buttonString.length > 0
			Layout.alignment: Qt.AlignCenter

			onClicked: buttonClicked();
		}
	}

	function selectItem(index: int): void {
		if (listview.currentIndex !== index) {
			const userrow = userModel.findUserByName(userModel.clientsNames[index]);
			if (userrow > 0) {
				clientRow = index;
				clientSelected(userrow);
				listview.currentIndex = index;
			}
		}
	}
}
