import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Pages

Rectangle {
	id: _button
	height: btnUp.height + btnDown.height
	width: btnUp.width
	color: "transparent"
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	focus: false
	x: _position.x
	y: _position.y

	signal scrollTo(int pos);

	property bool showUpButton: true
	property bool showDownButton: true
	property TPPage parentPage

//private:
	property point _position: Qt.point(parentPage.y + parentPage.height - height - 20, AppSettings.pageWidth - width - 20)
	property bool _visible: false
	readonly property int _button_size: AppSettings.itemLargeHeight

	Component.onCompleted: {
		parentPage.pageDeActivated.connect(function() { _visible = _button.visible; _button.visible = false; });
		parentPage.pageActivated.connect(function() { if (_visible) _button.visible = true; });
	}

	TPFloatingControl {
		id: btnUp
		parentPage: _button.parentPage
		width: _button._button_size
		height: _button._button_size
		radius: _button._button_size
		color: "transparent"
		visible: _button.visible && _button.showUpButton
		dragWidget: img1

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
		}

		onControlMoved: (xpos, ypos) => {
			_button.x = xpos;
			_button.y = ypos;
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
		parentPage: _button.parentPage
		width: _button._button_size
		height: _button._button_size
		radius: _button._button_size
		visible: _button.visible && _button.showDownButton
		color: "transparent"
		dragWidget: img2

		anchors {
			bottom: parent.bottom
			horizontalCenter: parent.horizontalCenter
		}

		onControlMoved: (xpos, ypos) => {
			_button.x = xpos;
			_button.y = ypos;
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
