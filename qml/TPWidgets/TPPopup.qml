import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import "../"

Popup {
	id: tpPopup
	objectName: "TPPopup"
	closePolicy: bKeepAbove ? Popup.NoAutoClose : Popup.CloseOnPressOutside
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0

	required property Page parentPage
	property bool bKeepAbove
	property bool bVisible: false
	property bool closeButtonVisible: true
	property int finalYPos: 0
	property int startYPos: 0

	onClosed: {
		bVisible = false;
		visible = false;
	}

	Component.onCompleted: {
		if (!modal && bKeepAbove) {
			parentPage.pageDeActivated.connect(function() { bVisible = tpPopup.visible; tpPopup.visible = false; });
			parentPage.pageActivated.connect(function() { if (bVisible) tpPopup.visible = true; });
		}
	}

	Rectangle {
		id: backRec
		implicitHeight: height
		implicitWidth: width
		radius: 8
		layer.enabled: true
		visible: false
		color: appSettings.primaryColor
	}

	background: backRec

	MultiEffect {
		id: backgroundEffect
		visible: true
		source: backRec
		anchors.fill: backRec
		shadowEnabled: true
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1
		opacity: 0.9
	}

	TPMouseArea {
		movableWidget: tpPopup
		movingWidget: backgroundEffect
	}

	contentItem {
		Keys.onPressed: (event) => {
			if (event.key === mainwindow.backKey) {
				event.accepted = true;
				close();
			}
		}
	}

	TPButton {
		imageSource: "close.png"
		hasDropShadow: false
		visible: closeButtonVisible
		height: 30
		width: 30

		z:2

		anchors {
			top: parent.top
			right: parent.right
		}

		onClicked: close();
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

	function show1(ypos: int): void {
		x = (appSettings.pageWidth - width)/2;

		if (ypos < 0)
			ypos = (appSettings.pageHeight - height)/2;

		finalYPos = ypos;
		if (ypos <= appSettings.pageHeight/2)
			startYPos = -300;
		else
			startYPos = appSettings.pageHeight + 300;

		open();
	}

	function show2(targetItem: Item, pos: int): void {
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
