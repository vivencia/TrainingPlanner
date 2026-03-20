import QtQuick
import QtQuick.Controls

import TpQml.Pages

Rectangle {
	id: _control
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	focus: false

//public:
	property TPPage parentPage
	property Item dragWidget

	signal clicked();
	signal controlMoved(int x, int y);

//private:
	property bool _visible: false

	Component.onCompleted: {
		parentPage.pageDeActivated.connect(function() { _visible = _control.visible; _control.visible = false; });
		parentPage.pageActivated.connect(function() { if (_visible) _control.visible = true; });
	}

	SequentialAnimation {
		id: anim
		alwaysRunToEnd: true

		// Expand the button
		PropertyAnimation {
			target: _control
			property: "scale"
			to: 1.5
			duration: 200
			easing.type: Easing.InOutCubic
		}

		// Shrink back to normal
		PropertyAnimation {
			target: _control
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: _control.clicked();
	}

	TPMouseArea {
		movingWidget: _control.dragWidget
		movableWidget: _control
		onMouseClicked: anim.start();
		onMoved: (x, y) => _control.controlMoved(x, y);
	}
}
