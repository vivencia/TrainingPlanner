import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Rectangle {
	id: button
	property color textColor: AppSettings.fontColor
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
	property int imageSize: 20
	property string imageSource
	property bool bPressed: false
	property bool bEmitSignal: false
	property bool bFollowParentsOpacity: false
	property var buttonImage: null;

	signal clicked(int clickid);

	focus: true
	border.color: flat ? "transparent" : AppSettings.fontColor
	radius: rounded ? height : 6
	opacity: bFollowParentsOpacity ? parent.opacity : 1
	color: AppSettings.primaryDarkColor

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
		if (imageSource.length > 0)
		{
			var component = Qt.createComponent("TPButtonImage.qml", Qt.Asynchronous);

			function finishCreation() {
				buttonImage = component.createObject(button,
					{imageSource: imageSource, bIconOnly: text.length === 0, textUnderIcon: textUnderIcon, size: imageSize});
				resizeButton();
				optionsEffect.visible = true;
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		else
			resizeButton();
	}

	MultiEffect {
		id: optionsEffect
		source: buttonImage
		anchors.fill: buttonImage
		saturation: button.enabled ? 0 : -1.0
		shadowEnabled: button.enabled
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1
		visible: false
	}

	property double fillPosition: 1
	Behavior on fillPosition {
		NumberAnimation {
			id: flash
			duration: 200
		}
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
						anchors.leftMargin = 2;
					}
					else {
						anchors.right = parent.right;
						anchors.rightMargin = 2;
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

	function resizeButton() {
		const fwidth = fontMetrics.boundingRect(text).width;
		const fheight = fontMetrics.boundingRect("TM").height;
		if (!fixedSize) {
			buttonText.width = fwidth + 5
			implicitWidth = fwidth + (imageSource.length > 1 ? textUnderIcon ? 10 : imageSize + 10 : 15);
			buttonText.height = fheight + 10
			implicitHeight = fheight + (imageSource.length > 1 ? textUnderIcon ? imageSize + 10 : 10 : 10);
		}
		else {
			if (button.width > 0) {
				if (fwidth >= button.width)
				{
					if (button.height === 0) {
						buttonText.width = button.width - 10;
						buttonText.height = fheight * 2;
						buttonText.wrapMode = Text.WordWrap;
						button.height = buttonText.height + (imageSource.length > 1 ? textUnderIcon ? imageSize + 10 : 10 : 10);

					}
					else
						buttonText.elide = Text.ElideMiddle;
				}
			}
		}
	}
} //Rectangle
