import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
	signal menuEntrySelected(id: int);

	id: menu
	closePolicy: Popup.NoAutoClose
	modal: false
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	implicitWidth: mainLayout.implicitWidth
	implicitHeight: mainLayout.implicitHeight

	property var entriesList: []

	background: Rectangle {
		id: background
		color: AppSettings.primaryColor
		opacity: 0.9
	}

	enter: Transition {
		NumberAnimation {
			target: background
			property: "opacity"
			from: 0
			to: 1
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	exit: Transition {
		id: closeTransition
		NumberAnimation {
			target: background
			property: "opacity"
			from: 1
			to: 0
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	ColumnLayout {
		id: mainLayout
		anchors.fill: parent
		spacing: 3

		TransparentButton {
			id: exportbtn
			text: "Export"
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"export.png"
			clickId: 0
			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 5
		}

		TransparentButton {
			id: importbtn
			text: "Import"
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"import.png"
			clickId: 1
			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 5
		}
	}

	Component.onDestruction: {
		for(var i = 0; i < entriesList.length; ++i)
			delete entriesList[i];
	}

	function addEntry(label: string, img: string, id: int) {
		var component = Qt.createComponent("TransparentButton.qml", Qt.Asynchronous);

		function finishCreation() {
			var button = component.createObject(mainLayout, { "text": label, "imageSource": "qrc:/images/"+AppSettings.iconFolder+img,  "clickId": id,
				"Layout.fillWidth": true, "Layout.leftMargin": 5, "Layout.rightMargin": 5 });
			button.buttonClicked.connect(menuEntrySelected);
			entriesList.push(button);
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function show(parent: Item, pos: int) {
		var point;
		switch (pos) {
			case 0: //top
				point = parent.mapTo(mainwindow, parent.y, parent.x);
				menu.x = point.x;
				menu.y = point.y - menu.height - 5
			break;
			case 1: //left
				point = parent.mapToGlobal(parent.y, parent.x + parent.width);
				menu.x = point.x + 5;
				menu.y = point.y;
			break;
			case 2: //right
				point = parent.mapToGlobal(parent.y, parent.x);
				menu.x = point.x - menu.width - 5;
				menu.y = point.y;
			break;
			case 2: //bottom
				point = parent.mapToGlobal(parent.y + parent.height, parent.x);
				menu.x = point.x;
				menu.y = point.y + 5;
			break;
		}
		menu.open();
	}
}
