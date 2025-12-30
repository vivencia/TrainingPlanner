import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import "../"

Popup {
	id: tpPopup
	closePolicy: keepAbove ? Popup.NoAutoClose : Popup.CloseOnPressOutside
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0

	required property Page parentPage
	property bool keepAbove: false
	property bool bVisible: false
	property bool closeButtonVisible: true
	property bool disableMouseHandling: false
	property bool showTitleBar: true
	property bool enableEffects: true
	property bool useBackgroundGradient: false
	property int finalYPos: 0
	property int startYPos: 0
	property double titleBarOpacity: 1
	property alias btnClose: btnCloseWindow
	property alias titleBar: titlebar
	property int _key_pressed
	property TPBackRec backgroundRec: _backRec

	readonly property int titleBarHeight: appSettings.itemDefaultHeight + 5
	signal keyboardNumberPressed(int key1, int key2);
	signal keyboardEnterPressed();
	signal backKeyPressed();

	onOpened: mainwindow.appPagesModel.popupOpened(this);
	onClosed: mainwindow.appPagesModel.popupClosed(this);

	Component.onCompleted: {
		if (!modal && keepAbove) {
			parentPage.pageDeActivated.connect(function() { bVisible = tpPopup.visible; tpPopup.visible = false; });
			parentPage.pageActivated.connect(function() { if (bVisible) tpPopup.visible = true; });
		}
	}

	TPBackRec {
		id: _backRec
		useGradient: useBackgroundGradient
		implicitHeight: height
		implicitWidth: width
		radius: 8
		visible: backgroundRec === this
	}

	Loader {
		asynchronous: true
		active: enableEffects

		RectangularShadow {
			x: backgroundRec.x
			y: backgroundRec.y
			width: backgroundRec.width
			height: backgroundRec.height
			color: "#80000000" // Semi-transparent black
			radius: 8
			cached: true
			offset: Qt.point(5, 5) // Horizontal and vertical offset
			spread: 5 // controls the shadow's expansion
		}
	}

	background: backgroundRec

	Timer {
		id: keyPressTimer
		interval: 800
	}

	contentItem {
		Keys.onPressed: (event) => {
			switch (event.key) {
				case Qt.Key_Enter:
				case Qt.Key_Return:
					keyboardEnterPressed();
				break;
				default:
					if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9)
					{
						if (keyPressTimer.running)
						{
							keyPressTimer.stop();
							keyboardNumberPressed(event.key, _key_pressed);
						}
						else
						{
							_key_pressed = event.key;
							keyboardNumberPressed(event.key, -1);
							keyPressTimer.start();
						}
					}
				break;
			}
		}
	}

	TPBackRec {
		id: titlebar
		useGradient: true
		radius: 8
		opacity: titleBarOpacity
		height: titleBarHeight
		visible: showTitleBar

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
		}
	}

	TPButton {
		id: btnCloseWindow
		imageSource: "close.png"
		hasDropShadow: false
		visible: closeButtonVisible
		width: appSettings.itemDefaultHeight
		height: width
		z: 2

		anchors {
			top: parent.top
			topMargin: 2
			right: parent.right
			rightMargin: 2
		}

		onClicked: closePopup();
	}

	TPMouseArea {
		enabled: !disableMouseHandling
		movableWidget: tpPopup
		movingWidget: titlebar
		onPositionChanged: (mouse) => positionChangedFunction(mouse);
		onPressed: mainwindow.appPagesModel.raisePopup(tpPopup);
	}

	enter: Transition {
		NumberAnimation {
			property: "y"
			from: startYPos
			to: finalYPos
			duration: 500
			easing.type: Easing.InOutCubic
		}
		NumberAnimation {
			property: "opacity"
			from: 0
			to: 0.9
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	exit: Transition {
		id: closeTransition
		NumberAnimation {
			property: "y"
			from: finalYPos
			to: startYPos
			duration: 500
			easing.type: Easing.InOutCubic
		}
		NumberAnimation {
			property: "opacity"
			from: 0.9
			to: 0
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	function closePopup(): void {
		bVisible = false;
		close();
	}

	function show1(ypos: int): void {
		if (visible)
			return;

		x = (appSettings.pageWidth - width)/2;

		if (ypos < 0) {
			switch (ypos) {
				case -1: ypos = (appSettings.windowHeight - height)/2; break;
				case -2: ypos = appSettings.windowHeight - height; break;
				case -3: ypos = y; break;
				case -4: ypos = appSettings.pageHeight - height; break;
			}
		}

		finalYPos = ypos;
		if (ypos <= appSettings.pageHeight/2)
			startYPos = -300;
		else
			startYPos = appSettings.pageHeight + 300;

		open();
	}

	function show2(targetItem: Item, pos: int): void {
		if (visible) {
			close();
			return;
		}
		const point = targetItem.parent.mapToItem(parent, targetItem.x, targetItem.y);;

		var xpos, ypos;
		switch (pos) {
			case 0: //top
				xpos = point.x;
				ypos = point.y - height - 15;
			break;
			case 1: //left
				xpos = point.x - width - 15;
				ypos = point.y;
			break;
			case 2: //right
				xpos = point.x + targetItem.width;
				ypos = point.y;
			break;
			case 3: //bottom
				xpos = point.x;
				ypos = point.y + targetItem.height;
			break;
		}

		if (xpos < 0)
			xpos = 0;
		else if (xpos + width > parentPage.width - 20)
			xpos = parentPage.width - width - 10;
		if (ypos < 0)
			ypos = 0;
		else if (ypos + height > parentPage.height)
			ypos = parentPage.height - height - 10;
		x = xpos;
		finalYPos = ypos;
		if (ypos > appSettings.pageHeight/2)
			startYPos = appSettings.pageHeight;
		open();
	}
}
