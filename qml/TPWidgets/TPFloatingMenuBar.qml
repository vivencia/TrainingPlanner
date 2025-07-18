import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"

TPPopup {
	id: menu
	keepAbove: false
	height: entriesTotalHeight
	width: largestEntryWidth
	closeButtonVisible: false

	property list<Item> entriesList: []
	property int entriesTotalHeight: 0
	property int largestEntryWidth: 0
	property Component entryComponent: null

	signal menuEntrySelected(id: int);

	Component.onDestruction: clear();

	ColumnLayout {
		id: mainLayout
		anchors.fill: parent
		spacing: 5
		opacity: menu.opacity
	}

	function addEntry(label: string, img: string, id: int, bvisible: bool): void {
		if (!entryComponent)
			entryComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPButton.qml", Qt.Asynchronous);

		function finishCreation() {
			let button = entryComponent.createObject(mainLayout, { text: label, imageSource: img, clickId: id, rounded: false,
									autoSize: true, color: "transparent", "Layout.fillWidth": true });
			if (bvisible) {
				entriesTotalHeight += button.height + 7;
				if (button.width > largestEntryWidth)
					largestEntryWidth = button.width + 10;
				button.clicked.connect(menuEntryClicked);
			}
			else
				button.visible = false;
			entriesList.push(button);
		}

		if (entryComponent.status === Component.Ready)
			finishCreation();
		else
			entryComponent.statusChanged.connect(finishCreation);
	}

	function clear(): void {
		for(let i = 0; i < entriesList.length; ++i)
			entriesList[i].destroy();
		entriesList.length = 0;
		entriesTotalHeight = 0;
		close();
	}

	function enableMenuEntry(id: int, benabled: bool): void {
		for(let i = 0; i < entriesList.length; ++i) {
			if (entriesList[i].clickId === id) {
				entriesList[i].enabled = benabled;
				return;
			}
		}
	}

	function setMenuText(id: int, newText: string): void {
		for(let i = 0; i < entriesList.length; ++i) {
			if (entriesList[i].clickId === id) {
				entriesList[i].text = newText;
				return;
			}
		}
	}

	function menuEntryClicked(buttonid: int): void {
		menuEntrySelected(buttonid);
		menu.close();
	}
}
