import QtQuick
import QtQuick.Controls

import ".."
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPBackRec {
	id: button
	focus: true
	border.color: flat ? "transparent" : buttonText.color
	radius: rounded ? height : 8
	opacity: checked ? 0.9 : 1
	color: backgroundColor
	height: autoSize ? buttonText.contentHeight : (text.length > 0 ? appSettings.itemDefaultHeight * buttonText.lineCount : 0) +
							(textUnderIcon ? imageLoader.width : 0)
	width: autoSize ? preferredWidth : undefined
	useGradient: enabled && button.text.length !== 0

	readonly property int preferredWidth: buttonText.contentWidth + (textUnderIcon ? 0 : appSettings.itemDefaultHeight) + (text.length > 0 ? 20 : 0)
	property color textColor: appSettings.fontColor
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string backgroundColor: text.length > 0 ? appSettings.paneBackgroundColor : "transparent"
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
				paneColor = appSettings.paneBackgroundColor;
				lightColor = appSettings.primaryLightColor;
				midColor = appSettings.primaryColor;
				darkColor = appSettings.primaryDarkColor;
			}
		}

		onTriggered: {
			switch (iteration) {
				case 4:
					paneColor = appSettings.primaryLightColor;
				break;
				case 3:
					paneColor = appSettings.paneBackgroundColor;
				break;
				case 2:
					midColor = appSettings.primaryLightColor;
				break;
				case 1:
					midColor = appSettings.primaryColor;
					darkColor = appSettings.primaryLightColor;
				break;
				case 0:
					highlightTimer.stop();
				return;
			}
			--iteration;
		}
	}

	Label {
		id: buttonText
		visible: text.length > 0
		color: enabled ? appSettings.fontColor : appSettings.disabledFontColor
		wrapMode: multiline ? Text.WordWrap : Text.NoWrap
		font: AppGlobals.regularFont
		minimumPixelSize: appSettings.smallFontSize * 0.8
		maximumLineCount: multiline ? 5 : 1
		fontSizeMode: autoSize ? Text.FixedSize : Text.Fit
		topInset: 0
		bottomInset: 0
		leftInset: 0
		rightInset: 0
		padding: 0
		opacity: button.opacity
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter

		Component.onCompleted: {
			if (textUnderIcon) {
				anchors.bottom = button.bottom;
				anchors.bottomMargin = 5;
				anchors.left = button.left;
				anchors.right = button.right;
			}
			else {
				if (imageSource.length > 0) {
					width = button.width - appSettings.itemDefaultHeight - 5;
					height = button.height - 5;
					anchors.horizontalCenter = button.horizontalCenter;
					anchors.verticalCenter = button.verticalCenter;
				}
				else
					anchors.fill = button;
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

	Loader {
		id: imageLoader
		active: imageSource.length > 0
		asynchronous: true
		sourceComponent: TPImage {
			source: imageSource
			dropShadow: hasDropShadow
			opacity: button.opacity
			enabled: checkable ? !checked : button.enabled
		}

		Component.onCompleted: {
			if (buttonText.text.length === 0) {
				anchors.fill = parent;
				anchors.margins = 2;
				flat = true;
			}
			else {
				if (textUnderIcon) {
					width = appSettings.itemDefaultHeight;
					anchors.top = button.top;
					anchors.topMargin = 5;
					anchors.horizontalCenter = button.horizontalCenter;
					anchors.bottomMargin = 10;
				}
				else {
					width = appSettings.itemDefaultHeight * 0.9;
					anchors.verticalCenter = button.verticalCenter;
					anchors.verticalCenterOffset = 2;
					if (iconOnTheLeft) {
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
