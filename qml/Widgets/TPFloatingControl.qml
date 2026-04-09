import QtQuick

import TpQml.Pages

Rectangle {
	id: _control

//public:
	property TPPage parentPage
	property Item dragWidget

	signal clicked();
	signal controlMoved(int x, int y);

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
