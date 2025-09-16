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
	readonly property int buttonSize: Math.ceil(userSettings.itemLargeHeight)

	TPFloatingControl {
		id: btnUp
		parentPage: ownerPage
		width: buttonSize
		height: buttonSize
		radius: buttonSize
		color: "transparent"
		visible: button.visible && showUpButton
		dragWidget: img1
		x: appSettings.pageWidth - buttonSize - 10;
		y: btnDown.y - buttonSize
		emitMoveSignal: true

		onControlMoved: (xpos, ypos) => {
			btnDown.x = xpos;
			btnDown.y = y + buttonSize;
		}

		TPImage {
			id: img1
			source: "upward"
			dropShadow: false
			anchors.fill: parent
		}

		onClicked: scrollTo(0);
	}

	TPFloatingControl {
		id: btnDown
		parentPage: ownerPage
		width: buttonSize
		height: buttonSize
		radius: buttonSize
		visible: button.visible && showDownButton
		color: "transparent"
		dragWidget: img2
		x: appSettings.pageWidth - buttonSize - 10;
		y: appSettings.pageHeight - buttonSize
		emitMoveSignal: true

		onControlMoved: (xpos, ypos) => {
			btnUp.x = xpos;
			btnUp.y = y - buttonSize;
		}

		TPImage {
			id: img2
			source: "downward"
			dropShadow: false
			anchors.fill: parent
		}

		onClicked: scrollTo(1);
	}
}
