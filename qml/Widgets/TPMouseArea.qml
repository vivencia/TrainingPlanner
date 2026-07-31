import QtQuick

import TpQml

MouseArea {
	id: _control
	propagateComposedEvents: true
	pressAndHoldInterval: slideToClose ? 100 : 300
	z: 1
	anchors.fill: movingWidget

//public:
	required property var movingWidget
	required property var movableWidget
	property bool lockMovingToYAxis: false
	property bool lockMovingToXAxis: false
	property bool slideToClose: false
	property Item viewPort: ItemManager.popupsVisualParent

	enum SlideToSide { MA_TOP, MA_BOTTOM, MA_LEFT, MA_RIGHT }

	signal mouseClicked(mouse: MouseEvent)
	signal mousePressed(mouse: MouseEvent)
	signal movingFinished(x: int, y: int)
	signal slideOutToSide(side: int)

//private:
	property point _mouse_pos_within_widget
	property point _last_moving_pos
	property bool _pressed: false
	property bool _pressed_and_held: false
	property bool _moved: false

	onReleased: (mouse) => {
		if (_pressed_and_held) {
			_pressed_and_held = false;
			mouse.accepted = true;
			if (_moved) {
				//Prevent the control from going out sight
				if (!lockMovingToYAxis) {
					if (movableWidget.x < 0)
						movableWidget.x = 0;
					else if (movableWidget.x + movableWidget.width > AppSettings.windowWidth)
						movableWidget.x = AppSettings.pageWidth - movableWidget.width;
				}
				if (!lockMovingToXAxis) {
					if (movableWidget.y < 0)
						movableWidget.y = 0;
					else if (movableWidget.y + movableWidget.height > AppSettings.windowHeight)
						movableWidget.y = AppSettings.windowHeight - movableWidget.height;
				}
				movingFinished(movableWidget.x, movableWidget.y);
				_moved = false;
			}
		} else if (_pressed) {
			_pressed = false;
			mouse.accepted = true;
			mouseClicked(mouse);
		}
	}

	onPressed: (mouse) => {
		_pressed = true;
		mouse.accepted = true;
		mousePressed(mouse);
	}

	onPressAndHold: (mouse) => {
		_pressed_and_held = true;
		_pressed = false;
		mouse.accepted = true;
		_mouse_pos_within_widget = movingWidget.mapToItem(movingWidget, mouse.x, mouse.y);
		if (slideToClose)
			_last_moving_pos = movingWidget.mapToItem(viewPort, mouse.x, mouse.y);
	}

	onPositionChanged: (mouse) => {
		if (_pressed_and_held) {
			if (!lockMovingToYAxis)
				movableWidget.x += mouse.x - _mouse_pos_within_widget.x;
			if (!lockMovingToXAxis)
				movableWidget.y += mouse.y - _mouse_pos_within_widget.y;
			_moved = true;
			mouse.accepted = true;
			if (slideToClose) {
				const mouse_pos = movingWidget.mapToItem(viewPort, mouse.x, mouse.y);
				const x_delta = _last_moving_pos.x - mouse_pos.x;
				if (Math.abs(x_delta) >= 20) {
					if ( x_delta > 0)
						slideOutToSide(TPMouseArea.MA_LEFT);
					else
						slideOutToSide(TPMouseArea.MA_RIGHT);
					_pressed_and_held = false;
					return;
				} else {
				   const y_delta = _last_moving_pos.y - mouse_pos.y;
				   if (Math.abs(y_delta) >= 20) {
					   if ( y_delta > 0)
						   slideOutToSide(TPMouseArea.MA_TOP);
					   else
						   slideOutToSide(TPMouseArea.MA_BOTTOM);
						_pressed_and_held = false;
						return;
					}
				}
				_last_moving_pos = mouse_pos;
			}
		} else {
			mouse.accepted = false;
		}
	}
}
