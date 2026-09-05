import QtQuick
import QtQuick.Controls

import TpQml

Item {
	id: _control
	anchors.fill: parent

//public:
	property Item currentItem
	property int currentIndex: -1
	property list<string> indicatorsColors
	property bool interactive: true
	property bool showIndicator: true
	property int workMargins: 10
	property int indicatorBottomMargin: 20

//private:
	property list<Item> _children
	property list<int> _item_cur_pos
	property int _leftmost_pos: -9999
	property int _rightmost_pos: 9999
	property int _snapping_threshold: width * 0.2 + workMargins
	property bool _ignore_x_change: false

	Component.onCompleted: if (currentIndex >= 0) init();
	onCurrentIndexChanged: if (currentIndex >= 0 && _leftmost_pos === -9999) init();
	onChildrenChanged: {
		if (currentIndex >= 0 && _leftmost_pos != -9999) {
			_children = 0;
			init();
		}
	}

	Timer {
		id: resumeMouseTimer
		interval: 500
		onTriggered: mouseArea.resumeMouseOperations();
	}

	TPMouseArea {
		id: mouseArea
		objectName: "X"
		movingWidget: _control
		movableWidget: _control.currentItem
		movableIsPopup: false
		movableWidgetCanGoOutsideBounds: true
		lockMovingToXAxis: true
		enabledCondition: _control.interactive
		z: -1
		onMovingFinished: (x,y) => {
			if ((x >= -_control._snapping_threshold && x <= _control._snapping_threshold)
					|| (x >= width - _control._snapping_threshold && x <= width + _control._snapping_threshold))
				snapCurrentItemIntoPosition();
		}
	}

	PageIndicator {
		id: indicator
		objectName: "X"
		count: _control._children.length
		currentIndex: _control.currentIndex
		visible: _control.showIndicator && count > 1
		anchors {
			bottom: parent.bottom
			bottomMargin: _control.indicatorBottomMargin
			horizontalCenter: parent.horizontalCenter
		}
		z: 2

		delegate: TPButton {
			id: delegate
			text: String(delegate.index + 1)
			clickId: index
			showBorder: index === indicator.currentIndex
			useGradient: false
			radius: width / 2
			opacity: index === indicator.currentIndex ? 1 : 0.7
			color: _control.indicatorsColors[index]
			width: AppSettings.itemSmallHeight
			height: width
			onClicked: (clickid) => _control.setCurrentIndex(clickid);

			required property int index
		}
	}

	function arrangePositions(): void {
		_item_cur_pos = [];
		for (let i = 0; i < _children.length; ++i)
			_item_cur_pos.push(i - currentIndex);
	}

	function setCurrentIndex(index: int): void {
		currentIndex = index;
		if (currentItem)
			currentItem.onXChanged.disconnect(moveItems);
		currentItem = _children[currentIndex];
		currentItem.onXChanged.connect(moveItems);
		arrangePositions();

		if (currentIndex === 0) {
			_item_cur_pos[_children.length-1] = -1;
			_rightmost_pos = _item_cur_pos[_children.length-2];
			_leftmost_pos = -1;
		}
		else if (currentIndex === _children.length - 1) {
			_item_cur_pos[0] = 1;
			_leftmost_pos = _item_cur_pos[1];
			_rightmost_pos = 1;
		} else {
			_leftmost_pos = _item_cur_pos[0];
			_rightmost_pos = _item_cur_pos[_children.length - 1];
		}
		currentItem.x = workMargins;
	}

	function init(): void {
		for (let _i = 0, _x = 0; _i < children.length; ++_i) {
			if (children[_i].objectName !== "X") {
				_children.push(children[_i]);
				_children[_x].width = width - 2 * workMargins;
				_children[_x].height = height;
				_children[_x].index = _x++;
			}
		}
		setCurrentIndex(currentIndex);
	}

	function snapCurrentItemIntoPosition(): void {
		mouseArea.stopMouseOperations();
		currentItem.x = workMargins;
		resumeMouseTimer.start();
	}

	function moveItems(): void {
		if (_ignore_x_change) return;
		if (mouseArea.xMovingDirection === TPMouseArea.MA_LEFT) {
			if (currentItem.x <= -width + _snapping_threshold) {
				_ignore_x_change = true;
				mouseArea.stopMouseOperations();
				switchToNextItem();
				return;
			}
		} else if (mouseArea.xMovingDirection === TPMouseArea.MA_RIGHT) {
			if (currentItem.x >= width - _snapping_threshold) {
				_ignore_x_change = true;
				mouseArea.stopMouseOperations();
				switchToNextItem();
				return;
			}
		}
		for (let i = 0; i < _children.length; ++i) {
			if (i !== currentIndex) {
				if (_item_cur_pos[i] < _item_cur_pos[currentIndex])
					_children[i].x = currentItem.x - (_item_cur_pos[currentIndex] - _item_cur_pos[i]) * width - (2 * workMargins);
				else
					_children[i].x = currentItem.x + (_item_cur_pos[i] - _item_cur_pos[currentIndex]) * width + (2 * workMargins);
			}
		}
	}

	function switchToNextItem(): void {
		currentItem.onXChanged.disconnect(moveItems);
		for (let i = 0; i < _children.length; ++i) {
			if (mouseArea.xMovingDirection === TPMouseArea.MA_LEFT) {
				if (_item_cur_pos[i] === _leftmost_pos)
					_item_cur_pos[i] = _rightmost_pos;
				else
					_item_cur_pos[i]--;
			} else {
				if (_item_cur_pos[i] === _rightmost_pos)
					_item_cur_pos[i] = _leftmost_pos;
				else
					_item_cur_pos[i]++;
			}
			if (_item_cur_pos[i] === 0)
				currentIndex = i;
		}
		_ignore_x_change = false;
		currentItem = _children[currentIndex];
		currentItem.onXChanged.connect(moveItems);
		currentItem.x = workMargins;
		mouseArea.enabled = true;
	}

	function item(index: int): Item {
		if (index >= 0 && index < _children.length)
			return _children[index];
		return null;
	}
}
