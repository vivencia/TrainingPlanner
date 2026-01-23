import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

TPPopup {
	id: menu
	keepAbove: false
	closeButtonVisible: false
	width: Math.max(lblTitle, mainLayout.childrenRect.width + 20)
	height: mainLayout.childrenRect.height + lblTitle.height + 20

	property string titleHeader
	property Component entryComponent: null

	signal menuEntrySelected(id: int);

	Component.onDestruction: clear();

	ListModel {
		id: entriesList
	}

	TPLabel {
		id: lblTitle
		text: titleHeader
		horizontalAlignment: Text.AlignHCenter
		visible: titleHeader.length > 0
		height: visible ? appSettings.itemDefaultHeight : 0
		font: AppGlobals.smallFont

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
		}
	}

	ColumnLayout {
		id: mainLayout
		spacing: 2
		opacity: menu.opacity
		uniformCellSizes: true

		anchors {
			top: titleHeader.length > 0 ? lblTitle.bottom : parent.top
			left: parent.left
			margins: 5
			topMargin: 10
		}

		Repeater {
			id: entriesRepeater
			model: entriesList.count

			TPButton {
				required property int index
				text: entriesList.get(index).Label
				imageSource: entriesList.get(index).Image
				clickId: entriesList.get(index).ClickId
				visible: entriesList.get(index).Visible
				rounded: false
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: Math.max(appSettings.pageWidth * 0.5, menu.width - 10)

				onClicked: menuEntryClicked(clickId);
			}
		}
	}

	function addEntry(label: string, img: string, id: int, bvisible: bool): void {		
		entriesList.append( {"Label": label, "Image": img, "ClickId": id, "Visible": bvisible} );
	}

	function clear(): void {
		entriesList.clear();
		close();
	}

	function enableMenuEntry(id: int, benabled: bool): void {
		entriesList.get(id).Visible = benabled;
	}

	function setMenuText(id: int, newText: string): void {
		entriesList.get(id).Label = newText;
	}

	function menuEntryClicked(buttonid: int): void {
		menuEntrySelected(buttonid);
		menu.close();
	}
}
