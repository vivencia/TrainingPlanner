import QtQuick

MouseArea {
	propagateComposedEvents: true
	pressAndHoldInterval: 300
	z: 1
	anchors.fill: movingWidget

//public:
	required property var movingWidget
	required property var movableWidget
	property bool lockMovingToYAxis: false

	signal mouseClicked(MouseEvent mouse);
	signal moved(int x, int y);
	signal movingFinished(int x, int y);

//private:
	property point _mouse_pos_within_widget
	property bool _pressed: false
	property bool _moved: false

	onReleased: (mouse) => {
		if (!_pressed) {
			mouse.accepted = false;
			mouseClicked(mouse);
		}
		else {
			_pressed = false;
			if (_moved) {
				movingFinished(movableWidget.x, movableWidget.y);
				_moved = false;
			}
			mouse.accepted = true;
		}
	}
	onClicked: (mouse) => mouse.accepted = false;
	onPressAndHold: (mouse) => {
		_pressed = true;
		mouse.accepted = true;
		_mouse_pos_within_widget = movingWidget.mapToItem(movingWidget, mouse.x, mouse.y);
	}

	onPositionChanged: (mouse) => {
		if (_pressed) {
			if (!lockMovingToYAxis)
				movableWidget.x += mouse.x - _mouse_pos_within_widget.x;
			movableWidget.y += mouse.y - _mouse_pos_within_widget.y;
			moved(movableWidget.x, movableWidget.y);
			_moved = true;
			mouse.accepted = true;
		}
		else
			mouse.accepted = false;
	}
}
