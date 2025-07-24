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
	height: buttonText.contentHeight + (textUnderIcon ? imageSize : 0) + (text.length > 0 ? buttonText.lineCount * 10 : 0)
	width: autoSize ? (buttonText.contentWidth + (textUnderIcon ? 0 : imageSize) + (text.length > 0 ? 20 : 0)) : undefined

	property color textColor: appSettings.fontColor
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string backgroundColor: text.length > 0 ? appSettings.paneBackgroundColor : "transparent"
	property string imageSource
	property bool autoSize: false
	property bool textUnderIcon: false
	property bool highlighted: false
	property bool flat: true
	property bool iconOnTheLeft: false
	property bool rounded: true
	property bool checkable: false
	property bool hasDropShadow: true
	property bool checked: false
	property int clickId: -1
	property int imageSize: 0

	//Local variables. Do not use outside this file
	property bool _bPressed: false
	property bool _bEmitSignal: false
	property TPButtonImage _buttonImage: null

	signal clicked(int clickid);
	signal check(int clickid);

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
				buttonText.height = appSettings.itemDefaultHeight;
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

	property double fillPosition: 1
	Behavior on fillPosition {
		NumberAnimation {
			id: flash
			duration: 200
		}
	}

	Label {
		id: buttonText
		visible: text.length > 0
		color: enabled ? appSettings.fontColor : appSettings.disabledFontColor
		wrapMode: Text.WordWrap
		font: AppGlobals.regularFont
		minimumPixelSize: appSettings.smallFontSize
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
		if (_bPressed) {
			_bEmitSignal = true;
			_bPressed = false;
		}
	}

	MouseArea {
		hoverEnabled: true
		anchors.fill: button
		enabled: button.enabled

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
			if (_bEmitSignal) {
				_bEmitSignal = false;
				button.clicked(button.clickId);
			}
		}
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
					imageSize = Math.ceil(appSettings.itemDefaultHeight * 0.9);
			}
			else {
				if (autoSize)
					imageSize = appSettings.appDefaultHeight * 0.9;
				else
					imageSize = Math.min(height, width) * 0.9;
			}
			_buttonImage = component.createObject(button,
				{ imageSource: imageSource, width: imageSize, height: imageSize, dropShadow: hasDropShadow});
			if (button.text.length === 0)
				_buttonImage.anchors.centerIn = button;
		}
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}
} //Rectangle
