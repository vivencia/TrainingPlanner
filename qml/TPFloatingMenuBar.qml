import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
	signal menuEntrySelected(id: int);

	id: menu
	closePolicy: Popup.CloseOnPressOutside
	modal: false
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	height: entriesTotalHeight

	property var entriesList: []
	property int entriesTotalHeight: 0
	property var entryComponent: null

	contentItem.Keys.onBackPressed: (event) => {
		event.accepted = true;
		close();
	}

	onAboutToShow: {
		for(var i = 0; i < entriesList.length; ++i)
			entriesList[i].initTransition();
	}

	onAboutToHide: {
		for(var i = 0; i < entriesList.length; ++i)
			entriesList[i].finishTransition();
	}

	background: Rectangle {
		id: background
		color: "transparent"
	}

	ColumnLayout {
		id: mainLayout
		anchors.fill: parent
		spacing: 0
		opacity: background.opacity
	}

	Component.onDestruction: {
		for(var i = 0; i < entriesList.length; ++i)
			delete entriesList[i];
	}

	function addEntry(label: string, img: string, id: int) {
		if (!entryComponent)
			entryComponent = Qt.createComponent("TPButton.qml", Qt.Asynchronous);

		function finishCreation() {
			var button = entryComponent.createObject(mainLayout, { text: label, imageSource: "qrc:/images/"+AppSettings.iconFolder+img,
				clickId: id, flat: true, "Layout.fillWidth": true, "Layout.leftMargin": 5, "Layout.rightMargin": 5 });
			entriesTotalHeight += button.buttonHeight;
			button.clicked.connect(menuEntryClicked);
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

	function show(targetItem: Item, pos: int) {
		if (visible) {
			close();
			return;
		}
		const point = targetItem.parent.mapToItem(parent, targetItem.x, targetItem.y);;

		var xpos, ypos;
		switch (pos) {
			case 0: //top
				xpos = point.x;
				ypos = point.y - height;
			break;
			case 1: //left
				xpos = point.x - width
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
		else if (xpos + width > parent.width - 20)
			xpos = parent.width - width - 10;
		if (ypos < 0)
			ypos = 0;
		else if (ypos + height > parent.height)
			ypos = parent.height - height - 10;
		x = xpos;
		y = ypos;
		open();
	}

	function menuEntryClicked(buttonid: int) {
		menuEntrySelected(buttonid);
		menu.close();
	}
}
