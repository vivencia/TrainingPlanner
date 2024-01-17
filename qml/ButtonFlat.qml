import QtQuick
import QtQuick.Controls

Rectangle {
	id: button
	property alias textColor: buttonText.color
	property alias imageMirror: buttonImage.mirror
	property alias imageSize: buttonImage.height
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string imageSource
	property bool bPressed: false
	property bool bEmitSignal: false
	signal clicked();

	border.color: "black"
	radius: 10
	opacity: button.enabled ? (bPressed ? 0.3 : 1) : 0.3
	implicitWidth: buttonText.contentWidth + (buttonImage.visible ? buttonImage.width + 10 : 10)
	implicitHeight: buttonText.height + 10

	property double fillPosition: !anim.running

	Behavior on fillPosition {
		NumberAnimation {
			id: flash
			duration: 300
		}
	}

	gradient: Gradient {
		orientation: Gradient.Horizontal
		GradientStop { position: 0.0;								color: primaryDarkColor }
		GradientStop { position: button.fillPosition - 0.001;		color: primaryLightColor }
		GradientStop { position: button.fillPosition + 0.001;		color: primaryColor }
		GradientStop { position: 1.0;								color: primaryDarkColor }
	}

	Label {
		id: buttonText
		opacity: button.enabled ? 1.0 : 0.3
		color: button.enabled ? "white" : "black"
		anchors.verticalCenter: parent.verticalCenter
		anchors.left: parent.left
		anchors.leftMargin: 5
		font.weight: Font.ExtraBold
		font.bold: true
		font.pixelSize: AppSettings.titleFontSizePixelSize
	}

	Image {
		id: buttonImage
		height: 20
		width: height
		fillMode: Image.PreserveAspectFit
		anchors.verticalCenter: parent.verticalCenter
		mirror: false
		source: imageSource
		visible: imageSource.length > 1
	}

	MouseArea {
		anchors.fill: parent
		onClicked: (mouse) => {
			if (!mouse.wasHeld)
				bEmitSignal = true;
			bPressed = false;
		}
		onPressed: (mouse) => {
			mouse.accepted = true;
			bPressed = true;
			anim.start();
		}
		onReleased: (mouse) => {
			mouse.accepted = true;
			bPressed = false;
		}
	}

	SequentialAnimation {
		id: anim

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
				button.clicked();
			}
		}
	}

	Component.onCompleted: {
		if (buttonText.text.length === 0)
			buttonImage.anchors.horizontalCenter = button.horizontalCenter
		else
			buttonImage.anchors.left = buttonText.right;
	}
} //Rectangle
