import QtQuick

import TpQml
import TpQml.Pages

Rectangle {
	id: _button
	height: btnUp.height + btnDown.height
	width: btnUp.width
	visible: false
	color: "transparent"

	signal scrollTo(int pos);

	property point position
	property bool showUpButton: true
	property bool showDownButton: true
	property TPPage ownerPage
	readonly property int buttonSize: AppSettings.itemLargeHeight

	TPFloatingControl {
		id: btnUp
		parentPage: _button.ownerPage
		width: _button.buttonSize
		height: _button.buttonSize
		radius: _button.buttonSize
		color: "transparent"
		visible: _button.visible && _button.showUpButton
		dragWidget: img1
		x: _button.position.x - _button.buttonSize - 10
		y: btnDown.y - _button.buttonSize

		onControlMoved: (xpos, ypos) => {
			btnDown.x = xpos;
			btnDown.y = y + _button.buttonSize;
		}

		TPImage {
			id: img1
			source: "upward"
			dropShadow: false
			anchors.fill: parent
		}

		onClicked: _button.scrollTo(0);
	}

	TPFloatingControl {
		id: btnDown
		parentPage: _button.ownerPage
		width: _button.buttonSize
		height: _button.buttonSize
		radius: _button.buttonSize
		visible: _button.visible && _button.showDownButton
		color: "transparent"
		dragWidget: img2
		x: _button.position.x - _button.buttonSize - 10
		y: _button.position.y - _button.buttonSize

		onControlMoved: (xpos, ypos) => {
			btnUp.x = xpos;
			btnUp.y = y - _button.buttonSize;
		}

		TPImage {
			id: img2
			source: "downward"
			dropShadow: false
			anchors.fill: parent
		}

		onClicked: _button.scrollTo(1);
	}
}
