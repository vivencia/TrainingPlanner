import QtQuick

MouseArea {
	anchors.fill: movingWidget

	required property var movingWidget
	required property var movableWidget
	property point mousePosWithinWidget
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

	onPressed: (mouse) => mousePosWithinWidget = movingWidget.mapToItem(movingWidget, mouse.x, mouse.y);

	function pressedFunction(mouse: MouseEvent): void {
		bPressed = true;
	}

	function positionChangedFunction(mouse: MouseEvent): void {
		if (bPressed) {
			bMoved = true;
			movableWidget.x += mouse.x - mousePosWithinWidget.x;
			movableWidget.y += mouse.y - mousePosWithinWidget.y;
			moved(movableWidget.x, movableWidget.y);
		}
	}
}
