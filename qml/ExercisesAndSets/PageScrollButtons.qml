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

	signal scrollTo(int pos);

	property point position
	property bool showUpButton: true
	property bool showDownButton: true
	property TPPage ownerPage
	readonly property int buttonSize: appSettings.itemLargeHeight

	TPFloatingControl {
		id: btnUp
		parentPage: button.ownerPage
		width: button.buttonSize
		height: button.buttonSize
		radius: button.buttonSize
		color: "transparent"
		visible: button.visible && button.showUpButton
		dragWidget: img1
		x: button.position.x - button.buttonSize - 10
		y: btnDown.y - button.buttonSize

		onControlMoved: (xpos, ypos) => {
			btnDown.x = xpos;
			btnDown.y = y + button.buttonSize;
		}

		TPImage {
			id: img1
			source: "upward"
			dropShadow: false
			anchors.fill: parent
		}

		onClicked: button.scrollTo(0);
	}

	TPFloatingControl {
		id: btnDown
		parentPage: button.ownerPage
		width: button.buttonSize
		height: button.buttonSize
		radius: button.buttonSize
		visible: button.visible && button.showDownButton
		color: "transparent"
		dragWidget: img2
		x: button.position.x - button.buttonSize - 10
		y: button.position.y - button.buttonSize

		onControlMoved: (xpos, ypos) => {
			btnUp.x = xpos;
			btnUp.y = y - button.buttonSize;
		}

		TPImage {
			id: img2
			source: "downward"
			dropShadow: false
			anchors.fill: parent
		}

		onClicked: button.scrollTo(1);
	}
}
