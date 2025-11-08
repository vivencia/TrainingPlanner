import QtQuick

MouseArea {
	anchors.fill: movingWidget
	z: 1
	propagateComposedEvents: true
	pressAndHoldInterval: 300

	required property var movingWidget
	required property var movableWidget
	property point mousePosWithinWidget
	property bool bPressed: false

	signal mouseClicked()
	signal moved(int x, int y)

	onReleased: (mouse) => {
		if (!bPressed) {
			mouse.accepted = false;
			mouseClicked();
		}
		else {
			bPressed = false;
			mouse.accepted = true;
		}
	}
	onClicked: (mouse) => mouse.accepted = false;
	onPressAndHold: (mouse) => {
		bPressed = true;
		mouse.accepted = true;
		mousePosWithinWidget = movingWidget.mapToItem(movingWidget, mouse.x, mouse.y);
	}

	function positionChangedFunction(mouse: MouseEvent): void {
		if (bPressed) {
			movableWidget.x += mouse.x - mousePosWithinWidget.x;
			movableWidget.y += mouse.y - mousePosWithinWidget.y;
			moved(movableWidget.x, movableWidget.y);
			mouse.accepted = true;
		}
		else
			mouse.accepted = false;
	}
}
