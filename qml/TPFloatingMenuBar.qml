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
	width: windowWidth * 0.5
	height: entriesTotalHeight

	property var entriesList: []
	property int entriesTotalHeight: 0
	property var entryComponent: null

	background: Rectangle {
		id: background
		color: AppSettings.primaryColor
	}

	enter: Transition {
		NumberAnimation {
			target: background
			property: "opacity"
			from: 0.0
			to: 0.9
			duration: 500
			easing.type: Easing.InOutCubic
		}
		NumberAnimation {
			target: background
			property: "scale"
			from: 0.0
			to: 1.0
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	exit: Transition {
		id: closeTransition
		NumberAnimation {
			target: background
			property: "opacity"
			from: 0.9
			to: 0.0
			duration: 500
			easing.type: Easing.InOutCubic
			alwaysRunToEnd: true
		}
		NumberAnimation {
			target: background
			property: "scale"
			from: 1.0
			to: 0.0
			duration: 500
			easing.type: Easing.InOutCubic
			alwaysRunToEnd: true
		}
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
			entryComponent = Qt.createComponent("TransparentButton.qml", Qt.Asynchronous);

		function finishCreation() {
			var button = entryComponent.createObject(mainLayout, { "text": label, "imageSource": "qrc:/images/"+AppSettings.iconFolder+img,  "clickId": id,
				"Layout.fillWidth": true, "Layout.leftMargin": 5, "Layout.rightMargin": 5 });
			entriesTotalHeight += button.height;
			button.buttonClicked.connect(menuEntryClicked);
			entriesList.push(button);
		}

		if (entryComponent.status === Component.Ready)
			finishCreation();
		else
			entryComponent.statusChanged.connect(finishCreation);
	}

	function show(parent: Item, pos: int) {
		if (menu.visible) {
			menu.close();
			return;
		}

		var point;
		switch (pos) {
			case 0: //top
				point = parent.mapToItem(mainwindow.contentItem, parent.x, parent.y);
				menu.x = parent.x;
				menu.y = point.y - parent.height;
			break;
			case 1: //left
				point = parent.mapToItem(mainwindow.contentItem, parent.x + parent.width, parent.y);
				menu.x = parent.x + parent.width;
				menu.y = point.y;
			break;
			case 2: //right
				point = parent.mapToItem(mainwindow.contentItem, parent.x, parent.y);
				menu.x = parent.x - menu.width - 5;
				menu.y = point.y;
			break;
			case 3: //bottom
				point = parent.mapToItem(mainwindow.contentItem, parent.x, parent.y + parent.height);
				menu.x = parent.x;
				menu.y = point.y + parent.height;
			break;
		}
		menu.open();
	}

	function menuEntryClicked(buttonid: int) {
		menuEntrySelected(buttonid);
		menu.close();
	}
}
