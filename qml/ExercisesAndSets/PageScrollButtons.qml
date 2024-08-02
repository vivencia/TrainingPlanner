import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../TPWidgets"

Rectangle {
	id: button
	height: btnUp.height + btnDown.height
	width: btnUp.width
	visible: false
	color: "transparent"

	signal scrollTo(int pos)

	property bool showUpButton: true
	property bool showDownButton: true
	property Page ownerPage

	TPFloatingControl {
		id: btnUp
		objName: "btnUp"
		parentPage: ownerPage
		width: 30
		height: 30
		radius: 30
		color: "transparent"
		visible: showUpButton
		dragWidget: img1
		x: windowWidth - width;
		y: 640 - 2*height - 10
		emitMoveSignal: true

		onControlMoved: (xpos, ypos) => {
			btnDown.x = xpos;
			btnDown.y = y + 30;
		}

		Image {
			id: img1
			source: "qrc:/images/"+darkIconFolder+"downward.png"
			anchors.fill: parent
			mirrorVertically: true
		}

		onClicked: {
			scrollTo(0);
			showUpButton = false;
			showDownButton = true;
		}
	}

	TPFloatingControl {
		id: btnDown
		objName: "btnDown"
		parentPage: ownerPage
		width: 30
		height: 30
		radius: 30
		visible: showDownButton
		color: "transparent"
		dragWidget: img2
		x: windowWidth - width;
		y: 640 - height - 10
		emitMoveSignal: true

		onControlMoved: (xpos, ypos) => {
			btnUp.x = xpos;
			btnUp.y = y - 30;
		}

		Image {
			id: img2
			source: "qrc:/images/"+darkIconFolder+"downward.png"
			anchors.fill: parent
		}

		onClicked: {
			scrollTo(1);
			showUpButton = true;
			showDownButton = false;
		}
	}
}
