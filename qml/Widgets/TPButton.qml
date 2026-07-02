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
	height: autoSize ? buttonText.contentHeight : (text.length > 0 ?
					AppSettings.itemDefaultHeight * buttonText.lineCount : 0) + (textUnderIcon ? imageLoader.width : 0)
	implicitWidth: preferredWidth
	useGradient: enabled && text.length !== 0

//public:
	property color textColor: AppSettings.fontColor
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string backgroundColor: text.length > 0 ? AppSettings.paneBackgroundColor : "transparent"
	property string imageSource
	property bool autoSize: false
	property bool textUnderIcon: false
	property bool flat: false
	property bool iconOnTheLeft: false
	property bool rounded: true
	property bool checkable: false
	property bool hasDropShadow: true
	property bool checked: false
	property bool multiline: false
	property int clickId: -1

	signal clicked(int clickid)
	signal check(int clickid)

//private:
	readonly property int preferredWidth: buttonText.contentWidth + (textUnderIcon ? 0 :
															AppSettings.itemDefaultHeight) + (text.length > 0 ? 20 : 0)
	property bool _bPressed: false
	property bool _do_text_layout: false
	property bool _do_image_layout: false
	property bool _first_layout_done: false

	Component.onCompleted: {
		_button.manageLayout(_do_text_layout, _do_image_layout);
		_first_layout_done = true;
	}

	onImageSourceChanged: {
		if (_first_layout_done)
			manageLayout(text.length > 0, imageSource.length > 0);
		else
			_do_image_layout = imageSource.length > 0;
	}

	onWidthChanged: if (_button._first_layout_done)
						_button.manageLayout(buttonText.text.length > 0, _button.imageSource.length > 0);

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
		onTextChanged: {
			//if the previous text was null, the image control will be anchored to fill the entire button. So we break those anchors
			if (_button._first_layout_done)
				imageLoader.anchors.fill = undefined;
			else
				_button._do_text_layout = buttonText.text.length > 0;
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
		width: AppSettings.itemDefaultHeight
		height: width
		sourceComponent: TPImage {
			source: _button.imageSource
			dropShadow: _button.hasDropShadow
			opacity: _button.opacity
			enabled: _button.checkable ? !_button.checked : _button.enabled
		}
	} //imageLoader

	function manageLayout(has_text: bool, has_image: bool): void {
		if (!has_text && !has_image)
			return;
		if (has_image && !has_text) {
			imageLoader.anchors.fill = _button;
			imageLoader.anchors.margins = 2;
			_button.flat = true;
		}
		else if (has_text && !has_image)
			buttonText.anchors.fill = _button;
		else {
			_button.flat = false;
			if (_button.textUnderIcon) {
				buttonText.anchors.bottom = _button.bottom;
				buttonText.anchors.bottomMargin = 5;
				buttonText.anchors.left = _button.left;
				buttonText.anchors.right = _button.right;
				imageLoader.width = AppSettings.itemDefaultHeight;
				imageLoader.anchors.top = _button.top;
				imageLoader.anchors.topMargin = 5;
				imageLoader.anchors.horizontalCenter = _button.horizontalCenter;
				imageLoader.anchors.bottomMargin = 10;
			}
			else {
				imageLoader.width = AppSettings.itemDefaultHeight * 0.9;
				imageLoader.anchors.verticalCenter = _button.verticalCenter;
				imageLoader.anchors.verticalCenterOffset = 2;
				buttonText.width = _button.width - imageLoader.width - 5;
				buttonText.height = _button.height - 5;
				buttonText.anchors.horizontalCenter = _button.horizontalCenter;
				buttonText.anchors.verticalCenter = _button.verticalCenter;
				if (_button.iconOnTheLeft) {
					buttonText.anchors.horizontalCenterOffset = imageLoader.width/2;
					imageLoader.anchors.right = buttonText.left;
				}
				else {
					buttonText.anchors.horizontalCenterOffset = -imageLoader.width/2;
					imageLoader.anchors.left = buttonText.right;
				}
			}
		}
	}
} //Rectangle
