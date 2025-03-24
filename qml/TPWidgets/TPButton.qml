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
	height: 25
	width: buttonText._textWidth + (textUnderIcon ? 0 : (imageSource.length > 0 ? imageSize : 0)) + 10

	property color textColor: appSettings.fontColor
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string backgroundColor: text.length > 0 ? appSettings.paneBackgroundColor : "transparent"
	property string imageSource

	property bool textUnderIcon: false
	property bool highlighted: false
	property bool fixedSize: false
	property bool flat: true
	property bool rightAlignIcon: false
	property bool rounded: true
	property bool checkable: false
	property bool hasDropShadow: true
	property bool checked: false
	property bool autoResize: false
	property int clickId: -1
	property int imageSize: height

	//Local variables. Do not use outside this file
	property bool _canResize: true
	property bool _bPressed: false
	property bool _bEmitSignal: false
	property TPButtonImage _buttonImage: null

	signal clicked(int clickid);
	signal check(int clickid);

	onImageSourceChanged: {
		if (_buttonImage)
			_buttonImage.imageSource = imageSource;
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

	//implicitHeight for layouts, height for other circumstances
	//The width of the button must be specified either by the layout(or anchors) or must be explicitly set, in which case
	//the property fixedSize must be set to true
	onWidthChanged: {
		if (!_canResize) {
			_canResize = true;
			return;
		}

		if (!fixedSize && width >= 20 && text.length > 0) {
			const fwidth = buttonText._textWidth;
			if (fwidth >= width) {
				buttonText.wrapMode = Text.WordWrap;
				buttonText.singleLine = false;
				buttonText.width = textUnderIcon ? width - 10 : width - (imageSource.length > 0 ? imageSize : 0) - 10;
				buttonText.lineCount = Math.ceil(fwidth/width) + 1;
				buttonText.height = buttonText.lineCount * buttonText._textHeight;
				if (buttonText.height > height)
					implicitHeight = height = textUnderIcon ? buttonText.height + imageSize : buttonText.height;
			}
		}
	}

	Component.onCompleted: {
		if (imageSource.length > 0)
		{
			let component = Qt.createComponent("TPButtonImage.qml", Qt.Asynchronous);

			function finishCreation() {
				_buttonImage = component.createObject(button,
					{imageSource: imageSource, width: imageSize, height: imageSize, dropShadow: hasDropShadow});
				anchorComponents();
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
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

		property bool bCompleted: false

		onSizeChanged: resize();

		Component.onCompleted: {
			bCompleted = true;
			resize();
			anchorComponents();
		}

		function resize() {
			if (!fixedSize) {
				_canResize = false;
				width = _textWidth;
				button.width = _textWidth + (textUnderIcon ? 10 : (imageSource.length > 0 ? imageSize : 0)) + 20
			}
			else {
				width = textUnderIcon ? button.width - 10 : button.width - (imageSource.length > 0 ? imageSize : 0) - 10;
				if (height > button.height)
					height = button.height;
			}
			if (!autoResize)
				heightAvailable = button.height - 10 - (imageSource.length > 1 ? imageSize : 0);
		}
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

	function anchorComponents(): void {
		if (imageSource.length > 0) {
			if (!buttonText.bCompleted && !_buttonImage)
				return;
			if (button.text.length === 0) {
				fixedSize = true;
				_buttonImage.anchors.centerIn = button;
				return;
			}

			if (!textUnderIcon) {
				buttonText.anchors.verticalCenter = button.verticalCenter;
				_buttonImage.anchors.verticalCenter = button.verticalCenter;

				buttonText.anchors.horizontalCenter = button.horizontalCenter;
				buttonText.anchors.verticalCenter = button.verticalCenter;
				if (!rightAlignIcon) {
					buttonText.anchors.horizontalCenterOffset = -imageSize/2;
					_buttonImage.anchors.left = buttonText.right;
				}
				else {
					buttonText.anchors.horizontalCenterOffset = imageSize/2;
					_buttonImage.anchors.right = buttonText.left;
				}
			}
			else {
				buttonText.anchors.horizontalCenter = button.horizontalCenter;
				buttonText.anchors.bottom = button.bottom;
				buttonText.anchors.bottomMargin = 5
				_buttonImage.anchors.top = button.top;
				_buttonImage.anchors.topMargin = 5;
				_buttonImage.anchors.horizontalCenter = button.horizontalCenter;
				_buttonImage.anchors.bottomMargin = 10;
			}
		}
		else {
			buttonText.anchors.horizontalCenter = button.horizontalCenter;
			buttonText.anchors.verticalCenter = button.verticalCenter;
		}
	}
} //Rectangle
