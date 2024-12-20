import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import "../"

Rectangle {
	id: button
	focus: true
	border.color: flat ? "transparent" : appSettings.fontColor
	radius: rounded ? height : 6
	opacity: checked ? 0.7 : 1
	color: backgroundColor

	property color textColor: appSettings.fontColor
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string backgroundColor: text.length > 0 ? appSettings.paneBackgroundColor : "transparent"
	property string imageSource

	property bool textUnderIcon: false
	property bool highlighted: false
	property bool fixedSize: false
	property bool flat: true
	property bool leftAlign: true
	property bool rounded: true
	property bool checkable: false
	property bool hasDropShadow: true
	property bool checked: false
	property bool autoResize: false
	property int clickId: -1
	property int imageSize: hasDropShadow ? 30 : 20
	property bool bPressed: false
	property bool bEmitSignal: false
	property Item associatedItem: null
	property TPButtonImage buttonImage: null;

	signal clicked(int clickid);
	signal check(int clickid);

	onImageSourceChanged: {
		if (buttonImage)
			buttonImage.imageSource = imageSource;
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

	Component.onCompleted: {
		if (imageSource.length > 0)
		{
			var component = Qt.createComponent("TPButtonImage.qml", Qt.Asynchronous);

			function finishCreation() {
				buttonImage = component.createObject(button,
					{imageSource: imageSource, bIconOnly: text.length === 0, textUnderIcon: textUnderIcon,
							width: imageSize, height: imageSize, dropShadow: hasDropShadow});
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		resizeButton();
	}

	property double fillPosition: 1
	Behavior on fillPosition {
		NumberAnimation {
			id: flash
			duration: 200
		}
	}

	TPLabel {
		id: buttonText
		opacity: button.opacity
		fontSizeMode: autoResize ? Text.Fit : Text.FixedSize
		color: button.enabled ? textColor : appSettings.disabledFontColor
		topPadding: textUnderIcon ? 10 : 5
		bottomPadding: 5
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter

		onSizeChanged: resizeButton();

		Component.onCompleted: {
			if (imageSource.length > 0) {
				if (!textUnderIcon) {
					anchors.verticalCenter = button.verticalCenter;
					if (leftAlign) {
						anchors.left = button.left;
						anchors.leftMargin = 5;
					}
					else {
						anchors.right = button.right;
						anchors.rightMargin = 5;
					}
				}
				else {
					anchors.horizontalCenter = button.horizontalCenter;
					anchors.bottom = button.bottom;
					anchors.bottomMargin = 5
				}
			}
			else {
				anchors.horizontalCenter = button.horizontalCenter;
				anchors.verticalCenter = button.verticalCenter;
			}
		}
	}

	function onMousePressed(mouse: MouseEvent): void {
		mouse.accepted = true;
		bPressed = true;
		forceActiveFocus();
		fillPosition = 0;
		if (!checkable)
			anim.start();
		else {
			checked = !checked;
			check(clickId);
		}
	}

	function onMouseReleased(mouse: MouseEvent): void {
		mouse.accepted = true;
		if (bPressed) {
			bEmitSignal = true;
			bPressed = false;
		}
	}

	MouseArea {
		hoverEnabled: true
		anchors.fill: button

		onPressed: (mouse) => onMousePressed(mouse);
		onReleased: (mouse) => onMouseReleased(mouse);
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
		if (text.length > 0)
		{
			if (!fixedSize) {
				const fwidth = buttonText._textWidth;
				const fheight = buttonText._textHeight;
				if (fwidth < width)
				{
					implicitWidth = fwidth + (imageSource.length > 1 ? (textUnderIcon ? 10 : imageSize + 10) : 15);
					implicitHeight = fheight + (imageSource.length > 1 ? (textUnderIcon ? imageSize : 5) : 5);
				}
				else
				{
					buttonText.wrapMode = Text.WordWrap;
					buttonText.singleLine = false;
					buttonText.widthAvailable = availableWidth;
					implicitWidth = buttonText._preferredWidth + (imageSource.length > 1 ? (textUnderIcon ? 10 : imageSize + 10) : 15);
					implicitHeight = buttonText._preferredHeight + (imageSource.length > 1 ? (textUnderIcon ? imageSize : 5) : 10);
				}
			}
			else
			{
				buttonText.width = button.width - 10;
				if (buttonText.height > button.height)
					buttonText.height = button.height;
				if (!autoResize)
					buttonText.heightAvailable = button.height - 10 - (imageSource.length > 1 ? imageSize : 0);
			}
		}
		else
			width = height = imageSize;
	}
} //Rectangle
