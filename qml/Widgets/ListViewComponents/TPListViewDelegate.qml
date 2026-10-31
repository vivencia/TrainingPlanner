pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

//This Component is not meant to be derived from. tpListView.delegateItem is the Item/Component that must be differentiated to
//interact with the model

SwipeDelegate {
	id: _control
	width: parent ? parent.width : 0
	height: _itemVisible ? preferredHeight : 0
	visible: _itemVisible
	spacing: 0

//public:
	required property TPListView tpListView
	required property Item itemContent
	readonly property TPListModel listModel: tpListView.listModel
	property int preferredHeight: itemContent.height
	readonly property bool currentItem: _control.listModel.currentRow === _control._index

	signal setupLayout()
	signal itemClicked(int index)

//protected:
	property int _index
	property bool _itemVisible
	property bool _set_up: false

	Behavior on height {
		id: heightBehavior
		enabled: false

		SpringAnimation {
			easing.type: Easing.InOutQuad
			spring: 3
			damping: 0.2
			mass: 2
		}
	}

	onCurrentItemChanged: {
		if (_set_up) {
			heightBehavior.enabled = true;
			if (currentItem)
				preferredHeight += 30;
			else
				preferredHeight -= 30;
			heightBehavior.enabled = false;
		}
	}

	onHeightChanged: {
		if (!_set_up) {
			if (bufferTimer.running)
				return;
			else
				bufferTimer.start();
		}
	}

	onClicked: {
		listModel.currentRow = _index;
		itemClicked(_index);
	}

	Connections {
		target: _control.listModel
		function onItemRemoved(item: int): void {
			if (item === _control._index)
				_control.tpListView.delegates_height -= _control.height - 2*_control.tpListView.spacing;
		}
		function onCancelRemove(): void {
			if (_control.listModel.currentRow === _control._index)
				_control.swipe.close();
		}
	}

	Timer {
		id: bufferTimer
		interval: 500
		onTriggered: {
			if (_control._index >= 0 && _control._itemVisible) //TPListView minimum size already accounts for the first item
				_control.tpListView.delegates_height += _control.height + 2*_control.tpListView.spacing;
			//Now that size reached its final value, layout the widgets inside listView.delegateItem. Only useful for complex
			//items like TPRadioButtonOrCheckBox or multi-items items.
			_control.setupLayout();
			_control._set_up = true;
		}
	}

	contentItem: _control.itemContent

	background: TPBackRec {
		id:	backgroundColor
		useGradient: true
		showBorder: _control.currentItem
		opacity: _control.currentItem ? 1.0 : 0.6
	}

	swipe.right: Rectangle {
		width: parent ? parent.width : 0
		height: parent ? parent.height: 0
		clip: false
		color: SwipeDelegate.pressed ? "#555" : "#666"
		radius: 8

		TPImage {
			source: "remove"
			width: _control.height >= AppSettings.itemLargeHeight ? _control.height / 2 : _control.height * 0.9
			height: width
			opacity: 2 * -_control.swipe.position

			anchors {
				right: parent.right
				rightMargin: 20
				verticalCenter: parent.verticalCenter
			}
		}
	} //swipe.right

	swipe.onCompleted: {
		_control.listModel.currentRow = _control._index;
		if (swipe.position === -1) { //swipe right
			if (_control.listModel.isSelected(_control._index))
				_control.listModel.removeSelected();
			else
				_control.listModel.removeItem(_control._index);
		}
	}
}
