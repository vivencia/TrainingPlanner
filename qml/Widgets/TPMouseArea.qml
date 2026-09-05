import QtQuick

import TpQml

MouseArea {
	id: _control
	propagateComposedEvents: true
	pressAndHoldInterval: canSlide ? 100 : 300
	enabled: enabledCondition
	z: 1
	anchors.fill: movingWidget

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
	readonly property Item viewPort: ItemManager.popupsVisualParent

	enum SlideToSide { MA_NONE, MA_TOP, MA_BOTTOM, MA_LEFT, MA_RIGHT }

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

	onClicked: (mouse) => mouse.accepted = false;

	onReleased: (mouse) => {
		if (_pressed_and_held) {
			_pressed_and_held = false;
			mouse.accepted = true;
			if (_moved) {
				if (!movableWidgetCanGoOutsideBounds) { //Prevent the control from going out sight
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
				}
				movingFinished(movableWidget.x, movableWidget.y);
				_moved = false;
			}
		} else if (_pressed) {
			_pressed = false;
			//mouse.accepted = false;
			mouseClicked(mouse);
		} else {
			mouse.accepted = false;
		}
	}

	onPressed: (mouse) => {
		_pressed = true;
		//mouse.accepted = false;
		mousePressed(mouse);
	}

	Component.onCompleted: {
		if (movableIsPopup) {
			onPressAndHold.connect(pressAndHold_popup);
			onPositionChanged.connect(positionChanged_popup);
		} else {
			onPressAndHold.connect(pressAndHold_item);
			onPositionChanged.connect(positionChanged_item);
		}
	}

	function pressAndHold_popup(mouse): void {
		_pressed_and_held = true;
		_pressed = false;
		mouse.accepted = true;
		_mouse_pos_within_widget = movingWidget.mapToItem(movingWidget, mouse.x, mouse.y);
		_last_moving_pos = movingWidget.mapToItem(viewPort, mouse.x, mouse.y);
	}
	function pressAndHold_item(mouse): void {
		_pressed_and_held = true;
		_pressed = false;
		mouse.accepted = true;
		_last_moving_pos = Qt.point(mouse.x, mouse.y);
	}

	function positionChanged_popup(mouse): void {
		if (_pressed_and_held) {
			const mouse_pos = movingWidget.mapToItem(viewPort, mouse.x, mouse.y);
			const x_delta = mouse_pos.x - _last_moving_pos.x;
			const y_delta = mouse_pos.y - _last_moving_pos.y;
			if (!lockMovingToYAxis) {
				if (x_delta < 0)
					xMovingDirection = TPMouseArea.MA_LEFT;
				else
					xMovingDirection = TPMouseArea.MA_RIGHT;
				movableWidget.x += mouse.x - _mouse_pos_within_widget.x;
			}
			if (!lockMovingToXAxis) {
				if ( y_delta < 0)
					yMovingDirection = TPMouseArea.MA_TOP;
				else
					yMovingDirection = TPMouseArea.MA_BOTTOM;
				movableWidget.y += mouse.y - _mouse_pos_within_widget.y;
			}
			_moved = true;
			mouse.accepted = true;
			_last_moving_pos = mouse_pos;
			if (canSlide) {
				if (!lockMovingToYAxis) {
					if (Math.abs(x_delta) >= 20) {
						slideOutToSide(xMovingDirection);
						_pressed_and_held = false;
						return;
					}
				}
				if (!lockMovingToXAxis) {
					if (Math.abs(y_delta) >= 20) {
						slideOutToSide(yMovingDirection);
						_pressed_and_held = false;
						return;
					}
				}
			}
		} else {
			mouse.accepted = false;
		}
	}
	function positionChanged_item(mouse): void {
		if (_pressed_and_held) {
			if (!lockMovingToYAxis) {
				if (mouse.x < _last_moving_pos.x)
					xMovingDirection = TPMouseArea.MA_LEFT;
				else
					xMovingDirection = TPMouseArea.MA_RIGHT;
				movableWidget.x += mouse.x - _last_moving_pos.x;
			}
			if (!lockMovingToXAxis) {
				if (mouse.y < _last_moving_pos.y)
					yMovingDirection = TPMouseArea.MA_TOP;
				else
					yMovingDirection = TPMouseArea.MA_BOTTOM;
				movableWidget.y += mouse.y - _last_moving_pos.y;
			}
			_moved = true;
			mouse.accepted = true;
			_last_moving_pos = Qt.point(mouse.x, mouse.y);
		} else {
			mouse.accepted = false;
		}
	}

	function stopMouseOperations(): void {
		enabled = false;
		_pressed = false;
		_pressed_and_held = false;
		_moved = false;
	}
	function resumeMouseOperations(): void {
		enabled = enabledCondition;
	}
}
