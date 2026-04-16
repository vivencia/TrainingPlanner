import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import TpQml
import TpQml.Pages

Popup {
	id: _control
	closePolicy: keepAbove ? Popup.NoAutoClose : Popup.CloseOnPressOutside
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0

//public:
	required property TPPage parentPage
	property bool keepAbove: false
	property bool closeButtonVisible: true
	property bool disableMouseHandling: false
	property bool showTitleBar: true
	property bool enableEffects: true
	property int finalYPos: 0
	property double titleBarOpacity: 1
	property alias btnClose: btnCloseWindow
	property alias titleBar: titlebar
	property TPBackRec backgroundRec: _backRec
	property string backGroundImage

	signal keyboardNumberPressed(int key1, int key2);
	signal keyboardEnterPressed();
	signal closeActionExeced();

//private:
	property int _start_y_pos: 0
	property int _key_pressed
	property bool _visible: false
	readonly property int titleBarHeight: AppSettings.itemDefaultHeight + 5

	onOpened: if (ItemManager.AppPagesManager) ItemManager.AppPagesManager.popupOpened(this);
	onClosed: if (ItemManager.AppPagesManager) ItemManager.AppPagesManager.popupClosed(this);

	Component.onCompleted: {
		if (!modal && keepAbove) {
			_control.parentPage.pageDeActivated.connect(function() { _control._visible = _control.visible; _control.visible = false; });
			_control.parentPage.pageActivated.connect(function() { if (_control._visible) _control.visible = true; });
		}
	}

	function changeParentPage(parent_page: Page): void {
		if (!modal && keepAbove) {
			parentPage.pageDeActivated.disconnect();
			parentPage.pageActivated.disconnect();
			parentPage = parent_page;
			parentPage.pageDeActivated.connect(function() { _visible = _control.visible; _control.visible = false; });
			parentPage.pageActivated.connect(function() { if (_visible) _control.visible = true; });
		}
	}

	TPBackRec {
		id: _backRec
		useGradient: _control.backGroundImage.length === 0
		useImage: _control.backGroundImage.length > 0
		sourceImage: _control.backGroundImage
		showBorder: true
		implicitHeight: height
		implicitWidth: width
		radius: 8
		layer.enabled: _control.enableEffects
		visible: true
	}

	Loader {
		asynchronous: true
		active: _control.enableEffects

		RectangularShadow {
			x: _control.backgroundRec.x
			y: _control.backgroundRec.y
			width: _control.backgroundRec.width
			height: _control.backgroundRec.height
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
				if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9) {
					if (keyPressTimer.running) {
						keyPressTimer.stop();
						keyboardNumberPressed(event.key, _key_pressed);
					}
					else {
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
		opacity: _control.titleBarOpacity
		height: _control.titleBarHeight
		visible: _control.showTitleBar

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
		visible: _control.closeButtonVisible
		width: AppSettings.itemDefaultHeight
		height: width
		z: 2

		anchors {
			verticalCenter: titlebar.verticalCenter
			right: titlebar.right
			rightMargin: 5
		}

		onClicked: _control.closePopup();
	}

	TPMouseArea {
		enabled: !_control.disableMouseHandling
		movableWidget: _control
		movingWidget: titlebar
		onPressed: ItemManager.AppPagesManager.raisePopup(_control);
	}

	enter: Transition {
		NumberAnimation {
			property: "y"
			from: _control._start_y_pos
			to: _control.finalYPos
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
			from: _control.finalYPos
			to: _control._start_y_pos
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
		_visible = false;
		closeActionExeced();
		close();
	}

	function showInWindow(pos: int): void {
		if (visible)
			return;

		let ypos = 0;
		let xpos = 0;
		if (pos < 0) {
			pos *= -1;
			if (pos & Qt.AlignTop)
				ypos = 0;
			else if (pos & Qt.AlignVCenter)
				ypos = (AppSettings.windowHeight - height) / 2;
			else if (pos & Qt.AlignBottom)
				ypos = AppSettings.windowHeight - height;
			if (pos & Qt.AlignHCenter)
				xpos = (AppSettings.pageWidth - width) / 2;
			else if (pos & Qt.AlignLeft)
				xpos = 0;
			else if (pos & Qt.AlignRight)
				xpos = AppSettings.windowWidth - width;
		}

		finalYPos = ypos;
		if (ypos <= AppSettings.windowHeight / 2)
			_start_y_pos = -height;
		else
			_start_y_pos = AppSettings.windowHeight + height;
		x = xpos;
		open();
	}

	function showByWidget(targetItem: Item, pos: int): void {
		if (visible) {
			close();
			return;
		}
		const point = targetItem.parent.mapToItem(parent, targetItem.x, targetItem.y);;

		var xpos, ypos;
		switch (pos) {
		case Qt.AlignTop:
			xpos = point.x;
			ypos = point.y - height;
			break;
		case Qt.AlignLeft:
			xpos = point.x - width - 20;
			ypos = point.y;
			break;
		case Qt.AlignRight:
			xpos = point.x + targetItem.width;
			ypos = point.y;
			break;
		case Qt.AlignBottom:
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
		if (ypos > AppSettings.pageHeight/2)
			_start_y_pos = AppSettings.pageHeight;
		open();
	}
}
