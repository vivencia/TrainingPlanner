import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"

TPPopup {
	signal menuEntrySelected(id: int);

	id: menu
	bKeepAbove: false
	height: entriesTotalHeight
	width: largestEntryWidth

	property list<Item> entriesList: []
	property int entriesTotalHeight: 0
	property int largestEntryWidth: 0
	property Component entryComponent: null

	Component.onDestruction: clear();

	ColumnLayout {
		id: mainLayout
		anchors.fill: parent
		spacing: 0
		opacity: menu.opacity
	}

	function addEntry(label: string, img: string, id: int, bvisible: bool): void {
		if (!entryComponent)
			entryComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPButton.qml", Qt.Asynchronous);

		function finishCreation() {
			var button = entryComponent.createObject(mainLayout, { text: label, imageSource: img, clickId: id,
				rounded: false, imageSize: 20, color: "transparent", "Layout.fillWidth": true });
			if (bvisible) {
				entriesTotalHeight += button.implicitHeight;
				if (button.implicitWidth > largestEntryWidth)
					largestEntryWidth = button.implicitWidth + 10;
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
		for(var i = 0; i < entriesList.length; ++i)
			entriesList[i].destroy();
		entriesList.length = 0;
		entriesTotalHeight = 0;
		close();
	}

	function enableMenuEntry(id: int, benabled: bool): void {
		if (id < entriesList.length)
			entriesList[id].enabled = benabled;;
	}

	function setMenuText(id: int, newText: string): void {
		if (id < entriesList.length)
			entriesList[id].text = newText;
	}

	function menuEntryClicked(buttonid: int): void {
		menuEntrySelected(buttonid);
		menu.close();
	}
}
