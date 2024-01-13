import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
	id: button
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates

	signal scrollTo(int pos)
	signal backButtonWasPressed()

	property bool showUpButton: true
	property bool showDownButton: true
	implicitHeight: btnUp.height + btnDown.height
	implicitWidth: btnUp.width
	color: "transparent"

	property bool bHeld: false
	property var prevPos

	Image {
		id: btnUp
		source: "qrc:/images/"+darkIconFolder+"downward.png"
		mirrorVertically: true
		width: 40
		height: 40
		visible: showUpButton

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		MouseArea {
			anchors.fill: parent

			onClicked: (mouse) => {
				if (!mouse.wasHeld) {
					scrollTo(0);
					showUpButton = false;
					showDownButton = true;
				}
			}

			onReleased: {
				bHeld = false;
			}

			onPressed: (mouse) => {
				prevPos = { x: mouse.x, y: mouse.y };
				bHeld = true;
			}

			onPositionChanged: {
				if (bHeld) {
					if (mouseX - prevPos.x < Math.abs(4)) {
						if (mouseY - prevPos.y < Math.abs(4)) {
							button.x += mouseX - prevPos.x;
							button.y += mouseY - prevPos.y;
						}
					}
					prevPos = { x: mouseX, y: mouseY };
				}
			}
		} //MouseArea
	}

	Image {
		id: btnDown
		source: "qrc:/images/"+darkIconFolder+"downward.png"
		width: 40
		height: 40
		visible: showDownButton

		anchors {
			bottom: parent.bottom
			left: parent.left
			right: parent.right
		}

		MouseArea {
			anchors.fill: parent

			onClicked: (mouse) => {
				if (!mouse.wasHeld) {
					scrollTo(1);
					showUpButton = true;
					showDownButton = false;
				}
			}

			onReleased: {
				bHeld = false;
			}

			onPressed: (mouse) => {
				prevPos = { x: mouse.x, y: mouse.y };
				bHeld = true;
			}

			onPositionChanged: {
				if (bHeld) {
					const deltaX = mouseX - prevPos.x;
					if (Math.abs(deltaX) < 10) {
						const deltaY = mouseY - prevPos.y;
						if (Math.abs(deltaY) < 10) {
							button.x += deltaX;
							button.y += deltaY;
						}
					}
					prevPos = { x: mouseX, y: mouseY };
				}
			}
		} //MouseArea
	}

	Component.onCompleted: {
		x = mainwindow.width - implicitWidth;
		y = mainwindow.height - implicitHeight - 10
		mainwindow.backButtonPressed.connect(maybeDestroy);
		mainwindow.mainMenuOpened.connect(hideButtons);
		mainwindow.mainMenuClosed.connect(showButtons);
	}

	function maybeDestroy() {
		if (button) {
			if (button.visible)
				button.destroy();
			button.backButtonWasPressed();
		}
	}

	function hideButtons() {
		button.visible = false;
	}

	function showButtons() {
		button.visible = true;
	}
}
