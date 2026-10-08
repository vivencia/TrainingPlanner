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
	readonly property int preferredHeight: itemContent.height

	signal setupLayout()

//protected:
	property int _index
	property bool _itemVisible

	onFocusChanged: {
		if (focus)
			listModel.currentRow = _index;
	}

	onHeightChanged: {
		if (bufferTimer.running)
			return;
		else
			bufferTimer.start();
	}

	Connections {
		target: _control.listModel
		function onItemRemoved(item: int): void {
			if (item === _control._index)
				_control.tpListView.delegates_height -= _control.height;
		}
	}

	Timer {
		id: bufferTimer
		interval: 500
		onTriggered: {
			if (_control._index > 0 && _control._itemVisible) //TPListView minimum size already accounts for the first item
				_control.tpListView.delegates_height += _control.height + 2*_control.tpListView.spacing;
			//Now that size reached its final value, layout the widgets inside listView.delegateItem. Only useful for complex
			//items like TPRadioButtonOrCheckBox or multi-items items.
			_control.setupLayout();
		}
	}

	contentItem: _control.itemContent

	background: TPBackRec {
		id:	backgroundColor
		useGradient: true
		showBorder: true
		opacity: 0.9
	}

	swipe.right: Rectangle {
		width: parent.width
		height: parent.height
		clip: false
		color: SwipeDelegate.pressed ? "#555" : "#666"
		radius: 5

		TPImage {
			source: "remove"
			width: AppSettings.itemDefaultHeight
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
		if (_control.listModel.isSelected(_control._index))
			_control.listModel.removeSelected();
		else
			_control.listModel.removeItem(_control._index);
	}
}
