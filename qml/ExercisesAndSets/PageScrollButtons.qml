import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../TPWidgets"
import "../Pages"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Rectangle {
	id: button
	height: btnUp.height + btnDown.height
	width: btnUp.width
	visible: false
	color: "transparent"

	signal scrollTo(int pos)

	property bool showUpButton: true
	property bool showDownButton: true
	property TPPage ownerPage

	TPFloatingControl {
		id: btnUp
		parentPage: ownerPage
		width: 30
		height: 30
		radius: 30
		color: "transparent"
		visible: button.visible && showUpButton
		dragWidget: img1
		x: appSettings.pageWidth - width;
		y: btnDown.y - height
		emitMoveSignal: true

		onControlMoved: (xpos, ypos) => {
			btnDown.x = xpos;
			btnDown.y = y + 30;
		}

		TPImage {
			id: img1
			source: "upward"
			dropShadow: false
			width: 30
			height: 30
		}

		onClicked: {
			scrollTo(0);
			showUpButton = false;
			showDownButton = true;
		}
	}

	TPFloatingControl {
		id: btnDown
		parentPage: ownerPage
		width: 30
		height: 30
		radius: 30
		visible: button.visible && showDownButton
		color: "transparent"
		dragWidget: img2
		x: appSettings.pageWidth - width;
		y: appSettings.pageHeight - 2*height
		emitMoveSignal: true

		onControlMoved: (xpos, ypos) => {
			btnUp.x = xpos;
			btnUp.y = y - 30;
		}

		TPImage {
			id: img2
			source: "downward"
			dropShadow: false
			width: 30
			height: 30
		}

		onClicked: {
			scrollTo(1);
			showUpButton = true;
			showDownButton = false;
		}
	}
}
