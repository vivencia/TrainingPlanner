import QtQuick

import TpQml

PointHandler {
	id: _control
	target: movingWidget
	grabPermissions: PointerHandler.TakeOverForbidden|PointerHandler.ApprovesTakeOverByItems
	enabled: enabledCondition

//public:
	required property var movingWidget
	required property var movableWidget
	property bool lockMovingToYAxis: false
	property bool lockMovingToXAxis: false
	property bool canSlide: false
	property bool movableWidgetCanGoOutsideBounds: false
	property bool enabledCondition: true
	property bool movableIsPopup: true
	property int xMovingDirection: TPMouseArea.MA_NONE
	property int yMovingDirection: TPMouseArea.MA_NONE

	enum SlideToSide { MA_NONE, MA_TOP, MA_BOTTOM, MA_LEFT, MA_RIGHT }

	signal movingFinished(x: int, y: int)
	signal slideOutToSide(side: int)
	signal widgetOutOfBounds()
	signal mousePressed()
	signal mouseReleased()
	signal mouseClicked()

//private:
	property point _last_moving_pos
	readonly property int _out_of_bounds_margin: AppSettings.itemDefaultHeight
	property point _movable_widget_pos
	property bool _long_pressed
	property Timer _long_press_timer: Timer {
		interval: 300
		onTriggered: {
			_control._last_moving_pos.x = _control.movableIsPopup ? _control.point.scenePosition.x : _control.point.position.x;
			_control._last_moving_pos.y = _control.movableIsPopup ? _control.point.scenePosition.y : _control.point.position.y;
			_control._movable_widget_pos = _control._last_moving_pos;
			_control._long_pressed = true;
		}
	}

	onGrabChanged: (transition, point) => {
		if (transition === PointerDevice.GrabPassive) {
			_long_pressed = false;
			_long_press_timer.start();
		} else if (transition === PointerDevice.UngrabPassive) {
			if (_long_pressed) {
				mousePressed();
				if (_movable_widget_pos !== _last_moving_pos)
					movingFinished(movableWidget.x, movableWidget.y);
			} else {
				mouseClicked();
			}
		}
	}

	onPointChanged: {
		if (!_long_pressed)
			return;
		if (point.pressedButtons) {
			let point_x = movableIsPopup ? point.scenePosition.x : point.position.x;
			let point_y = movableIsPopup ? point.scenePosition.y : point.position.y;
			const x_delta = point_x - _last_moving_pos.x;
			const y_delta = point_y - _last_moving_pos.y;
			if (!lockMovingToYAxis) {
				if (x_delta < 0)
					xMovingDirection = TPMouseArea.MA_LEFT;
				else
					xMovingDirection = TPMouseArea.MA_RIGHT;
				movableWidget.x += x_delta;
			}
			if (!lockMovingToXAxis) {
				if ( y_delta < 0)
					yMovingDirection = TPMouseArea.MA_TOP;
				else
					yMovingDirection = TPMouseArea.MA_BOTTOM;
				movableWidget.y += y_delta;
			}
			_last_moving_pos.x = point_x;
			_last_moving_pos.y = point_y;
			if (movableIsPopup) {
				if (canSlide) {
					if (!lockMovingToYAxis) {
						if (Math.abs(x_delta) >= 40) {
							slideOutToSide(xMovingDirection);
							return;
						}
					}
					if (!lockMovingToXAxis) {
						if (Math.abs(y_delta) >= 40) {
							slideOutToSide(yMovingDirection);
							return;
						}
					}
				}
			}
		} else {
			//The following code assumes movingWidget is a title bar-like object(i.e. extends from
			//left to right of movableWidget and is at the top. TODO: take into account the dimensions and position
			//of the movingWidget before deciding what the out of bounds margins are.
			let out_of_bounds = false;
			if (!lockMovingToYAxis) {
				if (movableWidget.x + movableWidget.width <= _out_of_bounds_margin) {
					out_of_bounds = true;
					if (!movableWidgetCanGoOutsideBounds)
						movableWidget.x = 0;
				} else if (movableWidget.x >= AppSettings.windowWidth - _out_of_bounds_margin) {
					out_of_bounds = true;
					if (!movableWidgetCanGoOutsideBounds)
						movableWidget.x = AppSettings.pageWidth - movableWidget.width;
				}
			}
			if (!lockMovingToXAxis) {
				if (movableWidget.y <= 0) {
					out_of_bounds = true;
					if (!movableWidgetCanGoOutsideBounds)
						movableWidget.y = 0;
				} else if (movableWidget.y >= AppSettings.windowHeight - _out_of_bounds_margin) {
					out_of_bounds = true;
					if (!movableWidgetCanGoOutsideBounds)
						movableWidget.y = AppSettings.windowHeight - movableWidget.height;
				}
			}
			if (out_of_bounds)
				widgetOutOfBounds();
		}
	}
}
