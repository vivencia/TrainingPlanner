import QtQuick

import TpQml

MouseArea {
	propagateComposedEvents: true
	pressAndHoldInterval: slideToClose ? 100 : 300
	z: 1
	anchors.fill: movingWidget

//public:
	required property var movingWidget
	required property var movableWidget
	property bool lockMovingToYAxis: false
	property bool slideToClose: false

	enum SlideToSide {
		MA_TOP, MA_BOTTOM, MA_LEFT, MA_RIGHT
	}

	signal mouseClicked(mouse: MouseEvent)
	signal mousePressed(mouse: MouseEvent)
	signal movingFinished(x: int, y: int)
	signal slideOutToSide(side: int)

//private:
	property point _mouse_pos_within_widget
	property point _last_moving_pos
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
	onPressed: (mouse) => mousePressed(mouse);

	onPressAndHold: (mouse) => {
		_pressed = true;
		mouse.accepted = true;
		_mouse_pos_within_widget = movingWidget.mapToItem(movingWidget, mouse.x, mouse.y);
		if (slideToClose)
			_last_moving_pos = movingWidget.mapToItem(ItemManager.AppHomePage(), mouse.x, mouse.y);
	}

	onPositionChanged: (mouse) => {
		if (_pressed) {
			if (!lockMovingToYAxis)
				movableWidget.x += mouse.x - _mouse_pos_within_widget.x;
			movableWidget.y += mouse.y - _mouse_pos_within_widget.y;
			_moved = true;
			mouse.accepted = true;
			if (slideToClose) {
				const mouse_pos = movingWidget.mapToItem(ItemManager.AppHomePage(), mouse.x, mouse.y);
				const x_delta = _last_moving_pos.x - mouse_pos.x;
				if (Math.abs(x_delta) >= 20) {
					if ( x_delta > 0)
						slideOutToSide(TPMouseArea.MA_LEFT);
					else
						slideOutToSide(TPMouseArea.MA_RIGHT);
					_pressed = false;
					return;
				}
				else {
				   const y_delta = _last_moving_pos.y - mouse_pos.y;
				   if (Math.abs(y_delta) >= 20) {
					   if ( y_delta > 0)
						   slideOutToSide(TPMouseArea.MA_TOP);
					   else
						   slideOutToSide(TPMouseArea.MA_BOTTOM);
						_pressed = false;
						return;
					}
				}
				_last_moving_pos = mouse_pos;
			}
		}
		else
			mouse.accepted = false;
	}
}
