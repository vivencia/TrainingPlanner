import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Pages

Rectangle {
	id: _control
	height: 2 * _button_size
	width: _button_size + 5
	color: "transparent"
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	focus: false
	x: _saved_pos.x
	y: _saved_pos.y

	signal scrollTo(int pos);

	property bool showUpButton: false
	property bool showDownButton: true
	property TPPage parentPage

//private:
	readonly property string _config_field_name: "navButton_" + parentPage.objectName
	readonly property point _saved_pos: AppSettings.getCustomValue(_config_field_name,
													Qt.point(AppSettings.pageWidth - width - 20, parentPage.height - height - 20))
	property bool _visible: false
	readonly property int _button_size: AppSettings.itemLargeHeight

	Component.onCompleted: {
		parentPage.pageDeActivated.connect(function() { _visible = _control.visible; _control.visible = false; });
		parentPage.pageActivated.connect(function() { if (_visible) _control.visible = true; });
	}

	TPMouseArea {
		movingWidget: _control
		movableWidget: _control
		onMouseClicked: (mouse)=> {
			if (mouse.y <= _control._button_size) {
				btnUp._bPressed = true;
				btnUp.onMouseReleased(mouse);
			} else {
				btnDown._bPressed = true;
				btnDown.onMouseReleased(mouse);
			}
		}
		onMovingFinished: (x, y) => AppSettings.setCustomValue(_control._config_field_name, Qt.point(x, y));
	}

	TPButton {
		id: btnUp
		imageSource: "upward"
		visible: _control.visible && _control.showUpButton
		focus: false
		width: _control._button_size
		height: _control._button_size
		radius: _control._button_size

		anchors {
			top: parent.top
			left: parent.left
		}

		onClicked: _control.scrollTo(0);
	}

	TPButton {
		id: btnDown
		imageSource: "downward"
		visible: _control.visible && _control.showDownButton
		focus: false
		width: _control._button_size
		height: _control._button_size
		radius: _control._button_size

		anchors {
			top: btnUp.bottom
			left: parent.left
		}

		onClicked: _control.scrollTo(1);
	}
}
