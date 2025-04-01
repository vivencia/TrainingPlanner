import QtQuick
import QtQuick.Controls

import "../"

Rectangle {
	id: control
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	focus: false

	property Page parentPage
	property var dragWidget
	property bool emitMoveSignal: false
	property string objName

	signal clicked()
	signal controlMoved(int xpos, int ypos)
	property bool bVisible: false

	Component.onCompleted: {
		parentPage.pageDeActivated.connect(function() { bVisible = control.visible; control.visible = false; });
		parentPage.pageActivated.connect(function() { if (bVisible) control.visible = true; });
	}

	SequentialAnimation {
		id: anim
		alwaysRunToEnd: true

		// Expand the button
		PropertyAnimation {
			target: control
			property: "scale"
			to: 1.5
			duration: 200
			easing.type: Easing.InOutCubic
		}

		// Shrink back to normal
		PropertyAnimation {
			target: control
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: clicked();
	}

	TPMouseArea {
		movingWidget: dragWidget
		movableWidget: control

		onPressed: (mouse) => pressedFunction(mouse);
		onPositionChanged: (mouse) => positionChangedFunction(mouse);
		onMouseClicked: anim.start();
		onMoved: (x, y) => {
			if (emitMoveSignal)
				controlMoved(x, y);
		}
	}
}
