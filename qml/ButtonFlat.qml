import QtQuick
import QtQuick.Controls

Rectangle {
	id: button
	property color textColor: "white"
	property alias imageMirror: buttonImage.mirror
	property alias imageSize: buttonImage.height
	property alias font: buttonText.font
	property alias text: buttonText.text
	property bool textUnderIcon: false
	property bool highlight: false
	property bool fixedSize: false

	property string imageSource
	property bool bPressed: false
	property bool bEmitSignal: false
	signal clicked();

	focus: true
	border.color: "black"
	radius: 6
	opacity: button.enabled ? (bPressed ? 0.3 : 1) : 0.3
	implicitWidth: fontMetrics.boundingRect(text).width + (imageSource.length > 1 ? textUnderIcon ? 10 : buttonImage.width + 10 : 10)
	implicitHeight: fontMetrics.boundingRect("TM").height + (imageSource.length > 1 ? textUnderIcon ? buttonImage.height + 10 : 10 : 10)

	onHighlightChanged: {
		if (highlight) {
			button.border.width = 2;
			anim2.start();
		}
		else {
			button.border.width = 1;
			anim2.stop();
		}
	}

	FontMetrics {
		id: fontMetrics
		font.family: buttonText.font.family
		font.pixelSize: AppSettings.fontSizeText
	}

	Component.onCompleted: {
		AppSettings.appFontSizeChanged.connect(resizeButton);
		resizeButton();
	}

	function resizeButton() {
		if (!fixedSize) {
			const fwidth = fontMetrics.boundingRect(text).width;
			buttonText.width = fwidth + 5
			implicitWidth = fwidth + (imageSource.length > 1 ? textUnderIcon ? 10 : buttonImage.width + 10 : 10);

			const fheight = fontMetrics.boundingRect("TM").height;
			buttonText.height = fheight + 10
			implicitHeight = fheight + (imageSource.length > 1 ? textUnderIcon ? buttonImage.height + 10 : 10 : 10);
		}
	}

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
		color: button.enabled ? textColor : "black"
		font.weight: Font.ExtraBold
		font.bold: true
		font.pixelSize: AppSettings.fontSizeText*0.9
		leftPadding: 5
		topPadding: textUnderIcon ? 10 : 5
		bottomPadding: 5
		rightPadding: 5

		onTextChanged: resizeButton();
		Component.onCompleted: {
			if (!textUnderIcon) {
				anchors.verticalCenter = button.verticalCenter;
				if (imageSource.length === 0)
					anchors.horizontalCenter = button.horizontalCenter;
			}
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
		width: 20
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
					anchors.leftMargin = 0;
				}
				else {
					anchors.top = button.top;
					anchors.topMargin = 5;
					anchors.horizontalCenter = button.horizontalCenter;
					anchors.bottomMargin = 10;
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
			button.forceActiveFocus();
			anim.start();
		}
		onReleased: (mouse) => {
			mouse.accepted = true;
			bPressed = false;
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
				button.clicked();
			}
		}
	}

	SequentialAnimation {
		id: anim2
		loops: Animation.Infinite
		alwaysRunToEnd: true

		PropertyAnimation {
			target: button
			property: "border.color"
			to: "gold"
			duration: 300
			easing.type: Easing.InOutCubic
		}

		PropertyAnimation {
			target: button
			property: "border.color"
			to: "black"
			duration: 300
			easing.type: Easing.InOutCubic
		}
	}
} //Rectangle
