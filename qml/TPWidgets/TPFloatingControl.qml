import QtQuick
import QtQuick.Controls

import "../"

Rectangle {
	id: control
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	focus: false
	opacity: mousearea.bMoved ? 0.5 : 0.9

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

	MouseArea {
		id: mousearea
		z: 1
		anchors.fill: dragWidget
		property point prevPos
		property bool bPressed: false
		property bool bMoved: false

		onReleased: {
			if (!bMoved)
				anim.start();
			bMoved = false;
			bPressed = false;
		}

		onPressed: (mouse) => {
			prevPos = { x: mouse.x, y: mouse.y };
			bPressed = true;
		}

		onPositionChanged: {
			if (bPressed) {
				bMoved = true;
				const deltaX = mouseX - prevPos.x;
				if (Math.abs(deltaX) < 10) {
					const deltaY = mouseY - prevPos.y;
					if (Math.abs(deltaY) < 10) {
						control.x += deltaX;
						control.y += deltaY;
						if (emitMoveSignal)
							controlMoved(control.x, control.y);
					}
				}
				prevPos = { x: mouseX, y: mouseY };
			}
		}
	}//MouseArea
}
