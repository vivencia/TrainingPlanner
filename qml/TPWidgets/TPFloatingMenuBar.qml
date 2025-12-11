import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"

TPPopup {
	id: menu
	keepAbove: false
	closeButtonVisible: false

	property list<Item> entriesList: []
	property Component entryComponent: null

	signal menuEntrySelected(id: int);

	Component.onDestruction: clear();

	ColumnLayout {
		id: mainLayout
		anchors.fill: parent
		spacing: 5
		opacity: menu.opacity

		Rectangle {
			Layout.fillHeight: true
			Layout.fillWidth: true
			border.color: "white"
			border.width: 2
			color: "transparent"
		}
	}

	function addEntry(label: string, img: string, id: int, bvisible: bool): void {
		if (!entryComponent)
			entryComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPButton.qml", Qt.Asynchronous);

		function finishCreation() {
			let button = entryComponent.createObject(mainLayout, { text: label, imageSource: img, clickId: id,
																		rounded: false, "Layout.fillWidth": true });
			if (bvisible)
				button.clicked.connect(menuEntryClicked);
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
