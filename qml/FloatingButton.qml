import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
	id: button
	color: bHeld ? buttonText.color : paneBackgroundColor
	radius: width / 2
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates

	property string text
	property string image
	property bool bHasText: text.length > 1
	property bool bHasImage: image.length > 1
	property int comboIndex
	property int nextSetNbr: 0

	property bool bHeld: false

	signal buttonClicked(int settype)

	property int textAndImageSize: (bHasText ? buttonText.width + (bHasImage ? buttonImage.width: 0) : bHasImage ? buttonImage.width: 0)

	implicitHeight: comboIndex <= 2 ? cboSetType.height : Math.max(buttonText.height, buttonImage.height) + 10;
	implicitWidth: 50 + (comboIndex <= 2 ? cboSetType.width + textAndImageSize : textAndImageSize)

	ComboBox {
		id: cboSetType
		model: [ { key:qsTr("Regular"), idx:0 }, { key:qsTr("Pyramid"), idx:1 }, { key:qsTr("Drop set"), idx:2 } ]
		width: 100
		currentIndex: comboIndex
		visible: comboIndex <= 2
		textRole: "key"
		valueRole: "idx"
		z: 0

		anchors {
			left: parent.left
			verticalCenter: parent.verticalCenter
			leftMargin: 25
		}

		onActivated: { comboIndex = parseInt(cboSetType.currentValue); }
		//onActivated: (index) => { comboIndex = index; }
	}

	Text {
		id: buttonText
		text: nextSetNbr >=2 ? button.text + " #" + nextSetNbr.toString() : button.text
		color: "white"
		padding: 0
		visible: bHasText
		z: 0

		anchors {
			left: comboIndex <= 2 ? cboSetType.right : parent.left
			verticalCenter: parent.verticalCenter
			leftMargin: 5
		}
	}

	Image {
		id: buttonImage
		visible: bHasImage
		source: "qrc:/images/"+lightIconFolder+image;
		width: bHasText > 1 ? 20 : 20
		height: bHasText > 1 ? 20 : 20
		z: 0
		anchors {
			right: parent.right
			rightMargin: 10
			verticalCenter: parent.verticalCenter
		}
	}

	MouseArea {
		property var prevPos
		anchors {
			top: parent.top
			bottom: parent.bottom
			left: comboIndex <= 2 ? cboSetType.right : parent.left
			right: parent.right
		}

		z: 1

		onClicked: (mouse) => {
			if (!mouse.wasHeld)
				buttonClicked(comboIndex);
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
	}

	Component.onCompleted: {
		x = mainwindow.width/2 - implicitWidth/2;
		y = mainwindow.height * 0.5 - implicitHeight;
		mainwindow.backButtonPressed.connect(maybeDestroy);
		mainwindow.mainMenuOpened.connect(hideButtons);
		mainwindow.mainMenuClosed.connect(showButtons);
	}

	function maybeDestroy() {
		if (button && button.visible)
			button.destroy();
	}

	function hideButtons() {
		button.visible = false;
	}

	function showButtons() {
		button.visible = true;
	}
}
