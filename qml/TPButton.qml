import QtQuick
import QtQuick.Controls

Rectangle {
	id: button
	property color textColor: AppSettings.fontColor
	property alias imageMirror: buttonImage.mirror
	property alias imageSize: buttonImage.height
	property alias font: buttonText.font
	property alias text: buttonText.text
	property bool textUnderIcon: false
	property bool highlighted: false
	property bool fixedSize: false
	property bool flat: true
	property bool leftAlign: true
	property bool rounded: true
	property alias buttonHeight: button.implicitHeight
	property int clickId: -1
	property string imageSource
	property bool bPressed: false
	property bool bEmitSignal: false

	signal clicked(int clickid);

	focus: true
	border.color: flat ? "transparent" : AppSettings.fontColor
	radius: rounded ? height : flat ? 0 : 6
	width: fontMetrics.boundingRect(text).width + (imageSource.length > 1 ? textUnderIcon ? 10 : buttonImage.width + 10 : 10)
	height: fontMetrics.boundingRect("TM").height + (imageSource.length > 1 ? textUnderIcon ? buttonImage.height + 10 : 10 : 10)

	SequentialAnimation {
		id: showTransition
		alwaysRunToEnd: true

		NumberAnimation {
			target: button
			property: "opacity"
			from: 0.0
			to: 1.0
			duration: 300
			easing.type: Easing.InOutCubic
		}
		NumberAnimation {
			target: button
			property: "scale"
			from: 0.0
			to: 1.0
			duration: 300
			easing.type: Easing.InOutCubic
		}
	}

	SequentialAnimation {
		id: hideTransition
		alwaysRunToEnd: true

		NumberAnimation {
			target: button
			property: "opacity"
			from: 1.0
			to: 0.0
			duration: 300
			easing.type: Easing.InOutCubic
			alwaysRunToEnd: true
		}
		NumberAnimation {
			target: button
			property: "scale"
			from: 1.0
			to: 0.0
			duration: 300
			easing.type: Easing.InOutCubic
			alwaysRunToEnd: true
		}
	}

	function initTransition() {
		showTransition.start();
	}

	function finishTransition() {
		hideTransition.start();
	}

	onHighlightedChanged:
		if (highlighted) {
			fillPosition = 0;
			highlighTimer.start();
		}
		else
			fillPosition = 1;

	Timer {
		id: highlighTimer
		running: false
		repeat: false
		interval: 200

		onTriggered: fillPosition = 1;
	}

	FontMetrics {
		id: fontMetrics
		font.family: buttonText.font.family
		font.pointSize: AppSettings.fontSizeText
	}

	Component.onCompleted: {
		AppSettings.appFontSizeChanged.connect(resizeButton);
		resizeButton();
	}

	function resizeButton() {
		const fwidth = fontMetrics.boundingRect(text).width;
		if (!fixedSize) {
			buttonText.width = fwidth + 5
			implicitWidth = fwidth + (imageSource.length > 1 ? textUnderIcon ? 10 : buttonImage.width + 10 : 15);

			const fheight = fontMetrics.boundingRect("TM").height;
			buttonText.height = fheight + 10
			implicitHeight = fheight + (imageSource.length > 1 ? textUnderIcon ? buttonImage.height + 10 : 10 : 10);
		}
		else {
			if (rounded) {
				buttonText.width = fwidth*0.9;
				buttonText.wrapMode = Text.WordWrap;
			}
		}
	}

	property double fillPosition: 1
	Behavior on fillPosition {
		NumberAnimation {
			id: flash
			duration: 200
		}
	}

	gradient: Gradient {
		orientation: Gradient.Horizontal
		GradientStop { position: 0.0;								color: AppSettings.primaryDarkColor }
		GradientStop { position: button.fillPosition - 0.001;		color: AppSettings.primaryLightColor }
		GradientStop { position: button.fillPosition + 0.001;		color: AppSettings.primaryColor }
		GradientStop { position: 1.0;								color: AppSettings.primaryDarkColor }
	}

	Label {
		id: buttonText
		opacity: button.opacity
		color: button.enabled ? textColor : AppSettings.disabledFontColor
		font.weight: Font.ExtraBold
		font.bold: true
		font.pointSize: AppSettings.fontSizeText*0.9
		leftPadding: 5
		topPadding: textUnderIcon ? 10 : 5
		bottomPadding: 5
		rightPadding: 5

		onTextChanged: resizeButton();

		Component.onCompleted: {
			if (imageSource.length > 0) {
				if (!textUnderIcon) {
					anchors.verticalCenter = button.verticalCenter;
					if (leftAlign) {
						anchors.left = button.left;
						anchors.leftMargin = rounded ? 10 : 2;
					}
					else {
						anchors.right = parent.right;
						anchors.rightMargin = rounded ? 10 : 2;
					}
				}
				else {
					anchors.horizontalCenter = button.horizontalCenter;
					anchors.bottom = button.bottom;
					anchors.bottomMargin = 2;
				}
			}
			else {
				anchors.horizontalCenter = button.horizontalCenter;
				anchors.verticalCenter = button.verticalCenter;
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
		opacity: button.opacity
		visible: imageSource.length > 1

		Component.onCompleted: {
			if (text.length > 0) {
				if (!textUnderIcon) {
					anchors.verticalCenter = button.verticalCenter;
					if (leftAlign) {
						anchors.left = buttonText.right
						anchors.leftMargin = 0;
					}
					else {
						anchors.right = buttonText.left
						anchors.rightMargin = 0;
					}
				}
				else {
					anchors.top = button.top;
					anchors.topMargin = 5;
					anchors.horizontalCenter = button.horizontalCenter;
					anchors.bottomMargin = 10;
				}
			}
			else {
				anchors.horizontalCenter = button.horizontalCenter
				anchors.verticalCenter = button.verticalCenter
			}
		}
	}

	MouseArea {
		anchors.fill: button
		enabled: button.enabled
		hoverEnabled: true

		onClicked: (mouse) => {
			if (enabled) {
				if (!mouse.wasHeld)
					bEmitSignal = true;
				bPressed = false;
			}
		}
		onPressed: (mouse) => {
			if (enabled) {
				mouse.accepted = true;
				bPressed = true;
				button.forceActiveFocus();
				button.fillPosition = 0;
				anim.start();
			}
		}
		onReleased: (mouse) => {
			mouse.accepted = true;
			bPressed = false;
		}

		onEntered: button.highlighted = true;
		onExited: button.highlighted = false;
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
				button.clicked(button.clickId);
			}
		}
	}
} //Rectangle
