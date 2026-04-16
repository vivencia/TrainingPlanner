pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml

TPBackRec {
	id: _button
	focus: true
	border.color: flat ? "transparent" : buttonText.color
	radius: rounded ? height : 8
	opacity: checked ? 0.9 : 1
	color: backgroundColor
	height: autoSize ? buttonText.contentHeight : (text.length > 0 ? AppSettings.itemDefaultHeight * buttonText.lineCount : 0) +
							(textUnderIcon ? imageLoader.width : 0)
	width: autoSize ? preferredWidth : undefined
	useGradient: enabled && buttonText.text.length !== 0

	readonly property int preferredWidth: buttonText.contentWidth + (textUnderIcon ? 0 :
																		AppSettings.itemDefaultHeight) + (text.length > 0 ? 20 : 0)
	property color textColor: AppSettings.fontColor
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string backgroundColor: text.length > 0 ? AppSettings.paneBackgroundColor : "transparent"
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
	property bool multiline: false
	property int clickId: -1

	//Local variables. Do not use outside this file
	property bool _bPressed: false

	signal clicked(int clickid);
	signal check(int clickid);

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
				_button.paneColor = AppSettings.paneBackgroundColor;
				_button.lightColor = AppSettings.primaryLightColor;
				_button.midColor = AppSettings.primaryColor;
				_button.darkColor = AppSettings.primaryDarkColor;
			}
		}

		onTriggered: {
			switch (iteration) {
			case 4: _button.paneColor = AppSettings.primaryLightColor; break;
			case 3: _button.paneColor = AppSettings.paneBackgroundColor; break;
			case 2: _button.midColor = AppSettings.primaryLightColor; break;
			case 1:
				_button.midColor = AppSettings.primaryColor;
				_button.darkColor = AppSettings.primaryLightColor;
				break;
			case 0: highlightTimer.stop(); return;
			}
			--iteration;
		}
	}

	Label {
		id: buttonText
		visible: text.length > 0
		color: enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
		wrapMode: _button.multiline ? Text.WordWrap : Text.NoWrap
		font: AppGlobals.regularFont
		minimumPixelSize: AppSettings.smallFontSize * 0.8
		maximumLineCount: _button.multiline ? 5 : 1
		fontSizeMode: _button.autoSize ? Text.FixedSize : Text.Fit
		topInset: 0
		bottomInset: 0
		leftInset: 0
		rightInset: 0
		padding: 0
		opacity: _button.opacity
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter

		Component.onCompleted: {
			if (_button.textUnderIcon) {
				anchors.bottom = _button.bottom;
				anchors.bottomMargin = 5;
				anchors.left = _button.left;
				anchors.right = _button.right;
			}
			else {
				if (_button.imageSource.length > 0) {
					width = _button.width - AppSettings.itemDefaultHeight - 5;
					height = _button.height - 5;
					anchors.horizontalCenter = _button.horizontalCenter;
					anchors.verticalCenter = _button.verticalCenter;
				}
				else
					anchors.fill = _button;
			}
		}
	}

	function onMousePressed(mouse: MouseEvent): void {
		mouse.accepted = true;
		_bPressed = true;
	}

	function onMouseReleased(mouse: MouseEvent): void {
		mouse.accepted = true;
		if (_bPressed) {
			if (checkable) {
				checked = !checked;
				check(clickId);
			}
			else
				anim.start();
			_bPressed = false;
		}
	}

	MouseArea {
		hoverEnabled: _button.text.length > 0
		anchors.fill: _button
		enabled: _button.enabled

		onPressed: (mouse) => _button.onMousePressed(mouse);
		onReleased: (mouse) => { if (containsMouse) _button.onMouseReleased(mouse); }
		onEntered: if (_button.text.length > 0) _button.highlighted = true;
		onExited: if (_button.text.length > 0) _button.highlighted = false;
	}

	SequentialAnimation {
		id: anim
		alwaysRunToEnd: true

		// Expand the button
		PropertyAnimation {
			target: _button
			property: "scale"
			to: 1.5
			duration: 200
			easing.type: Easing.InOutCubic
		}

		// Shrink back to normal
		PropertyAnimation {
			target: _button
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: _button.clicked(_button.clickId);
	}

	Loader {
		id: imageLoader
		active: _button.imageSource.length > 0
		asynchronous: true
		sourceComponent: TPImage {
			source: _button.imageSource
			dropShadow: _button.hasDropShadow
			opacity: _button.opacity
			enabled: _button.checkable ? !_button.checked : _button.enabled
		}

		onLoaded: {
			if (buttonText.text.length === 0) {
				anchors.fill = parent;
				anchors.margins = 2;
				_button.flat = true;
			}
			else {
				if (_button.textUnderIcon) {
					width = AppSettings.itemDefaultHeight;
					anchors.top = _button.top;
					anchors.topMargin = 5;
					anchors.horizontalCenter = _button.horizontalCenter;
					anchors.bottomMargin = 10;
				}
				else {
					width = AppSettings.itemDefaultHeight * 0.9;
					anchors.verticalCenter = _button.verticalCenter;
					anchors.verticalCenterOffset = 2;
					if (_button.iconOnTheLeft) {
						buttonText.anchors.horizontalCenterOffset = width/2;
						anchors.right = buttonText.left;
					}
					else {
						buttonText.anchors.horizontalCenterOffset = -width/2;
						anchors.left = buttonText.right;
					}
				}
				height = width;
			}
		}
	} //Loader
} //Rectangle
