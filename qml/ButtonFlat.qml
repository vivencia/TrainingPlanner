import QtQuick
import QtQuick.Controls

Rectangle {
	id: button
	property alias textColor: buttonText.color
	property alias imageMirror: buttonImage.mirror
	property alias imageSize: buttonImage.height
	property alias font: buttonText.font
	property alias text: buttonText.text
	property bool textUnderIcon: false

	property string imageSource
	property bool bPressed: false
	property bool bEmitSignal: false
	signal clicked();

	border.color: "black"
	radius: 6
	opacity: button.enabled ? (bPressed ? 0.3 : 1) : 0.3
	implicitWidth: buttonText.contentWidth + (buttonImage.visible ? textUnderIcon ? 10 : buttonImage.width + 10 : 10)
	implicitHeight: buttonText.height + (buttonImage.visible ? textUnderIcon ? buttonImage.height + 15 : 10 : 10)

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
		font.weight: Font.ExtraBold
		font.bold: true
		font.pixelSize: AppSettings.titleFontSizePixelSize
		padding: 0

		Component.onCompleted: {
			anchors.left = button.left;
			anchors.leftMargin = 5;
			if (!textUnderIcon)
				anchors.verticalCenter = button.verticalCenter;
			else {
				anchors.horizontalCenter = button.horizontalCenter;
				anchors.bottom = button.bottom;
				anchors.bottomMargin = 2;
			}
		}
	}

	Image {
		id: buttonImage
		height: 20
		width: height
		fillMode: Image.PreserveAspectFit
		mirror: false
		source: imageSource
		visible: imageSource.length > 1

		Component.onCompleted: {
			if (buttonText.text.length === 0) {
				anchors.horizontalCenter = button.horizontalCenter
				anchors.verticalCenter = button.verticalCenter
			}
			else {
				if (!textUnderIcon) {
					anchors.verticalCenter = button.verticalCenter;
					anchors.left = buttonText.right
					anchors.leftMargin = 2;
				}
				else {
					anchors.top = button.top;
					anchors.topMargin = 5;
					anchors.horizontalCenter = button.horizontalCenter;
					anchors.bottomMargin = 2;
				}
			}
		}
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
} //Rectangle
