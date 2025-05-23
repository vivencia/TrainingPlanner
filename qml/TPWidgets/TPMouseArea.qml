import QtQuick

MouseArea {
	id: mousearea
	z: 0
	anchors.fill: movingWidget

	required property var movingWidget
	required property var movableWidget
	property point prevPos
	property bool bPressed: false
	property bool bMoved: false

	signal mouseClicked()
	signal moved(int x, int y)

	onReleased: {
		if (!bMoved)
			mouseClicked();
		bMoved = false;
		bPressed = false;
	}

	function pressedFunction(mouse: MouseEvent): void {
		prevPos = { x: mouse.x, y: mouse.y };
		bPressed = true;
	}

	function positionChangedFunction(mouse: MouseEvent): void {
		if (bPressed) {
			bMoved = true;
			const deltaX = mouse.x - prevPos.x;
			if (Math.abs(deltaX) < 10) {
				const deltaY = mouse.y - prevPos.y;
				if (Math.abs(deltaY) < 10) {
					movableWidget.x += deltaX;
					movableWidget.y += deltaY;
					moved(movableWidget.x, movableWidget.y);
				}
			}
			prevPos = { x: mouse.x, y: mouse.y };
		}
	}
}
