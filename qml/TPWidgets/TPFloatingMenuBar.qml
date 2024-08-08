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

	property var entriesList: []
	property int entriesTotalHeight: 0
	property int largestEntryWidth: 0
	property var entryComponent: null

	Component.onDestruction: {
		for(var i = 0; i < entriesList.length; ++i)
			delete entriesList[i];
	}

	ColumnLayout {
		id: mainLayout
		anchors.fill: parent
		spacing: 0
		opacity: menu.opacity
	}

	function addEntry(label: string, img: string, id: int, bvisible: bool) {
		if (!entryComponent)
			entryComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPButton.qml", Qt.Asynchronous);

		function finishCreation() {
			var button = entryComponent.createObject(mainLayout, { text: label, imageSource: img, clickId: id,
				rounded: false, color: "transparent", "Layout.fillWidth": true });
			if (bvisible) {
				entriesTotalHeight += button.buttonHeight;
				if (button.implicitWidth > largestEntryWidth)
					largestEntryWidth = button.implicitWidth;
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

	function enableMenuEntry(id: int, benabled: bool) {
		entriesList[id].enabled = benabled;;
	}

	function setMenuText(id: int, newText: string) {
		entriesList[id].text = newText;
	}

	function show(targetItem: Item, pos: int) {
		const point = targetItem.parent.mapToItem(parent, targetItem.x, targetItem.y);;

		var xpos, ypos;
		switch (pos) {
			case 0: //top
				xpos = point.x;
				ypos = point.y - entriesTotalHeight - 15;
			break;
			case 1: //left
				xpos = point.x - largestEntryWidth - 15;
				ypos = point.y;
			break;
			case 2: //right
				xpos = point.x + targetItem.width;
				ypos = point.y;
			break;
			case 3: //bottom
				xpos = point.x;
				ypos = point.y + targetItem.height;
			break;
		}

		if (xpos < 0)
			xpos = 0;
		else if (xpos + largestEntryWidth > parent.width - 20)
			xpos = parent.width - largestEntryWidth - 10;
		if (ypos < 0)
			ypos = 0;
		else if (ypos + entriesTotalHeight > parent.height)
			ypos = parent.height - entriesTotalHeight - 10;
		x = xpos;
		finalYPos = ypos;
		if (ypos > windowHeight/2)
			startYPos = windowHeight;
		open();
	}

	function menuEntryClicked(buttonid: int) {
		menuEntrySelected(buttonid);
		menu.close();
	}
}
