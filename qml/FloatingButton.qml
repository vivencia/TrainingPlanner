import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
	id: button
	color: paneBackgroundColor
	opacity: bHeld ? 0.7 : 1
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
	property bool bEmitSignal: false

	property int textAndImageSize: (bHasText ? buttonText.width + (bHasImage ? buttonImage.width: 0) : bHasImage ? buttonImage.width: 0)

	implicitHeight: comboIndex <= 2 ? cboSetType.height : Math.max(buttonText.height, buttonImage.height) + 10;
	implicitWidth: 50 + (comboIndex <= 2 ? cboSetType.width + textAndImageSize : textAndImageSize)

	ToolButton {
		id: btnClose
		width: 20
		height: 20

		anchors {
			left: parent.left
			verticalCenter: parent.verticalCenter
			leftMargin: 5
		}
		onClicked: bFloatButtonVisible = false;

		Image {
			anchors.fill: parent
			source: "qrc:/images/"+lightIconFolder+"close.png";
		}
	}

	TPComboBox {
		id: cboSetType
		model: [ { text:qsTr("Regular"), value:0 }, { text:qsTr("Pyramid"), value:1 }, { text:qsTr("Drop set"), value:2 } ]
		width: 100
		currentIndex: comboIndex
		visible: comboIndex <= 2
		z: 0

		anchors {
			left: parent.left
			verticalCenter: parent.verticalCenter
			leftMargin: 27
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
			left: comboIndex <= 2 ? cboSetType.right : btnClose.right
			right: parent.right
		}

		z: 1

		onClicked: (mouse) => {
			if (!mouse.wasHeld)
				bEmitSignal = true;
		}

		onReleased: {
			bHeld = false;
		}

		onPressed: (mouse) => {
			prevPos = { x: mouse.x, y: mouse.y };
			bHeld = true;
			anim.start();
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

	SequentialAnimation {
		id: anim
		alwaysRunToEnd: true

		// Expand the button
		PropertyAnimation {
			target: button
			property: "scale"
			to: 1.5
			duration: 200
			easing.type: Easing.InOutCubic
		}

		// Shrink back to normal
		PropertyAnimation {
			target: button
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: {
			if (bEmitSignal) {
				bEmitSignal = false;
				buttonClicked(comboIndex);
			}
		}
	}

	Component.onCompleted: {
		x = (windowWidth - implicitWidth)/2;
		y = windowHeight * 0.5 - implicitHeight;
		mainwindow.backButtonPressed.connect(maybeDestroy);
		mainwindow.mainMenuOpened.connect(hideButtons);
		mainwindow.mainMenuClosed.connect(showButtons);
	}

	function maybeDestroy() {
		if (button && button.visible)
			button.destroy();
	}

	function hideButtons() {
		if (button)
			button.visible = false;
	}

	function showButtons() {
		if (button)
			button.visible = true;
	}
}
