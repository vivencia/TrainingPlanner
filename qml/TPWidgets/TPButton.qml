import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import "../"

Rectangle {
	id: button
	focus: true
	border.color: flat ? "transparent" : buttonText.color
	radius: rounded ? height : 6
	opacity: checked ? 0.7 : 1
	color: backgroundColor
	height: (autoSize ? buttonText.contentHeight : userSettings.itemDefaultHeight) +
							(textUnderIcon ? imageSize : 0) + (text.length > 0 ? buttonText.lineCount * 10 : 0)
	width: autoSize ? preferredWidth : undefined

	readonly property int preferredWidth: buttonText.contentWidth + (textUnderIcon ? 0 : imageSize) + (text.length > 0 ? 20 : 0)
	property color textColor: userSettings.fontColor
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string backgroundColor: text.length > 0 ? userSettings.paneBackgroundColor : "transparent"
	property string imageSource
	property bool autoSize: false
	property bool textUnderIcon: false
	property bool highlighted: false
	property bool flat: false
	property bool iconOnTheLeft: false
	property bool rounded: true
	property bool checkable: false
	property bool hasDropShadow: true
	property bool checked: false
	property bool multiline: true
	property int clickId: -1
	property int imageSize: 0

	//Local variables. Do not use outside this file
	property bool _bPressed: false
	property TPButtonImage _buttonImage: null

	signal clicked(int clickid);
	signal check(int clickid);

	property color color1: userSettings.paneBackgroundColor
	property color color2: userSettings.primaryLightColor
	property color color3: userSettings.primaryColor
	property color color4: userSettings.primaryDarkColor

	Gradient {
		id: enabledGradient
		orientation: Gradient.Horizontal
		GradientStop { position: 0.0; color: color1; }
		GradientStop { position: 0.25; color: color2; }
		GradientStop { position: 0.50; color: color3; }
		GradientStop { position: 0.75; color: color4; }
	}

	gradient: enabled ? enabledGradient : null

	onHighlightedChanged:
		if (highlighted)
			highlightTimer.start();
		else
			highlightTimer.stop();

	Timer {
		id: highlightTimer
		running: false
		repeat: true
		interval: 50

		property int iteration: 4

		onRunningChanged: {
			if (!running) {
				iteration = 4;
				color1 = userSettings.paneBackgroundColor;
				color2 = userSettings.primaryLightColor;
				color3 = userSettings.primaryColor;
				color4 = userSettings.primaryDarkColor;
			}
		}

		onTriggered: {
			switch (iteration) {
				case 4:
					color1 = userSettings.primaryLightColor;
				break;
				case 3:
					color1 = userSettings.paneBackgroundColor;
				break;
				case 2:
					color3 = userSettings.primaryLightColor;
				break;
				case 1:
					color3 = userSettings.primaryColor;
					color4 = userSettings.primaryLightColor;
				break;
				case 0:
					highlightTimer.stop();
				return;
			}
			--iteration;
		}
	}

	//implicitHeight for layouts, height for other circumstances
	//The width of the button must be specified either by the layout(or anchors) or must be explicitly set, in which case
	//the property fixedSize must be set to true

	onWidthChanged: {
		if (width < 0) return;
		if (!autoSize && text.length > 0)
			buttonText.width = Math.min(button.width*0.9, buttonText.width);
	}

	onHeightChanged: {
		if (height < 0) return;
		if (!autoSize && textUnderIcon) {
			if (buttonText.lineCount > 1)
				buttonText.height = button.height - imageSize - 10;
			else
				buttonText.height = userSettings.itemDefaultHeight;
		}
	}

	Component.onCompleted: {
		if (imageSource.length > 0)
			createImageComponent();
	}

	onImageSourceChanged: {
		if (imageSource.length === 0)
		{
			if (_buttonImage) {
				_buttonImage.destroy();
				_buttonImage = 0;
			}
		}
		else {
			if (_buttonImage)
				_buttonImage.imageSource = button.imageSource;
		}
	}

	Label {
		id: buttonText
		visible: text.length > 0
		color: enabled ? userSettings.fontColor : userSettings.disabledFontColor
		wrapMode: multiline ? Text.WordWrap : Text.NoWrap
		font: AppGlobals.regularFont
		minimumPixelSize: userSettings.smallFontSize
		maximumLineCount: 5
		fontSizeMode: autoSize ? Text.FixedSize : Text.Fit
		topInset: 0
		bottomInset: 0
		leftInset: 0
		rightInset: 0
		padding: 2
		opacity: button.opacity
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter

		Component.onCompleted: anchorLabel();
	}

	function onMousePressed(mouse: MouseEvent): void {
		mouse.accepted = true;
		_bPressed = true;
	}

	function onMouseReleased(mouse: MouseEvent): void {
		mouse.accepted = true;
		if (_bPressed) {
			if (checkable){
				checked = !checked;
				check(clickId);
			}
			else
				anim.start();
			_bPressed = false;
		}
	}

	MouseArea {
		hoverEnabled: button.text.length > 0
		anchors.fill: button
		enabled: button.enabled

		onPressed: (mouse) => onMousePressed(mouse);
		onReleased: (mouse) => { if (containsMouse) onMouseReleased(mouse); }
		onEntered: if (button.text.length > 0) button.highlighted = true;
		onExited: if (button.text.length > 0) button.highlighted = false;
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

		onFinished: button.clicked(button.clickId);
	}

	function anchorLabel(): void {
		if (textUnderIcon) {
			buttonText.anchors.bottom = button.bottom;
			buttonText.anchors.bottomMargin = 5;
			buttonText.anchors.left = button.left;
			buttonText.anchors.right = button.right;
		}
		else {
			if (imageSource.length > 0)
				buttonText.anchors.horizontalCenter = button.horizontalCenter;
			else
				buttonText.anchors.fill = button;
			buttonText.anchors.verticalCenter = button.verticalCenter;
		}
		if (_buttonImage)
			anchorImage();
	}

	function anchorImage(): void {
		if (textUnderIcon) {
			_buttonImage.anchors.top = button.top;
			_buttonImage.anchors.topMargin = 5;
			_buttonImage.anchors.horizontalCenter = button.horizontalCenter;
			_buttonImage.anchors.bottomMargin = 10;
		}
		else {
			_buttonImage.anchors.verticalCenter = button.verticalCenter;
			if (iconOnTheLeft) {
				buttonText.anchors.horizontalCenterOffset = imageSize/2;
				_buttonImage.anchors.right = buttonText.left;
			}
			else {
				buttonText.anchors.horizontalCenterOffset = -imageSize/2;
				_buttonImage.anchors.left = buttonText.right;
			}
		}
	}

	function createImageComponent(): void {
		let component = Qt.createComponent("TPButtonImage.qml", Qt.Asynchronous);

		function finishCreation() {
			if (text.length > 0) {
				imageSize = Math.min(buttonText.height, buttonText.width);
				if (imageSize === 0)
					imageSize = Math.ceil(userSettings.itemSmallHeight);
			}
			else {
				if (autoSize)
					imageSize = userSettings.appDefaultHeight * 0.9;
				else
					imageSize = Math.min(height, width) * 0.9;
			}
			_buttonImage = component.createObject(button,
				{ imageSource: imageSource, width: imageSize, height: imageSize, dropShadow: hasDropShadow});
			if (button.text.length === 0) {
				_buttonImage.anchors.centerIn = button;
				flat = true;
			}
		}
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}
} //Rectangle
