import QtQuick

MouseArea {
	id: movableRegion
	anchors.fill: movingWidget
	z: 1

	required property Item movingWidget
	property point prevPos

	onPressed: (mouse) => {
		prevPos = { x: mouse.x, y: mouse.y };
	}

	onPositionChanged: {
		const deltaX = mouseX - prevPos.x;
		if (Math.abs(deltaX) < 10) {
			const deltaY = mouseY - prevPos.y;
			if (Math.abs(deltaY) < 10) {
				parent.x += deltaX;
				parent.y += deltaY;
			}
		}
		prevPos = { x: mouseX, y: mouseY };
	}
}
