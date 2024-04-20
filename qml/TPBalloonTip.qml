import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
	property string message: ""
	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string imageSource: ""
	property string backColor: AppSettings.paneBackgroundColor
	property string textColor: "white"
	property bool highlightMessage: false
	property int startYPosition: 0

	property int finalXPos: 0
	property int finalYPos: 0
	property int startYPos: 0

	signal button1Clicked();
	signal button2Clicked();

	id: balloon
	closePolicy: Popup.NoAutoClose
	modal: false
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	width: windowWidth * 0.7
	height: lblTitle.height + lblMessage.height + Math.max(btn1.height, btn2.height) + 10

	background: Rectangle {
		id: background
		color: backColor
		radius: 8
	}

	NumberAnimation {
		id: alternateCloseTransition
		target: balloon
		alwaysRunToEnd: true
		running: false
		property: "x"
		from: x
		to: finalXPos
		duration: 500
		easing.type: Easing.InOutCubic
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
			to: 1
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
			from: 1
			to: 0
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	FontMetrics {
		id: fontMetrics
		font.family: lblMessage.font.family
		font.pixelSize: AppSettings.fontSize
	}

	Label {
		id: lblTitle
		text: title
		color: textColor
		wrapMode: Text.WordWrap
		elide: Text.ElideRight
		font.pixelSize: AppSettings.fontSizeTitle * 0.6
		font.weight: Font.Black
		width: parent.width - 20
		height: visible ? 30 : 0
		visible: title.length > 0
		padding: 0

		anchors {
			left: parent.left
			top: parent.top
			leftMargin: 10
			topMargin: 5
		}
	}

	Image {
		id: imgElement
		source: imageSource
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		visible: imageSource.length > 0
		width: 50
		height: 50

		Component.onCompleted: {
			anchors.left = parent.left;
			anchors.leftMargin = 5;
			if (lblTitle.visible)
				anchors.top = lblTitle.bottom;
			else
				anchors.verticalCenter = parent.verticalCenter;
		}
	}

	Label {
		id: lblMessage
		text: message
		color: textColor
		wrapMode: Text.WordWrap
		horizontalAlignment: Text.AlignJustify
		font.pixelSize: AppSettings.fontSize
		font.weight: Font.Black
		width: (imgElement.visible ? balloon.width - imgElement.width : balloon.width) - 20
		height: Math.ceil(fontMetrics.boundingRect(message).width / balloon.width) * 30
		visible: message.length > 0
		padding: 0

		anchors {
			left: imgElement.visible ? imgElement.right : parent.left
			top: lblTitle.visible ? lblTitle.bottom : parent.top
			leftMargin: 10
			topMargin: lblTitle.visible ? 5 : 10
		}
	}

	SequentialAnimation {
		loops: Animation.Infinite
		running: highlightMessage

		ColorAnimation {
			target: lblMessage
			property: "color"
			from: "white"
			to: "darkred"
			duration: 700
			easing.type: Easing.InOutCubic
		}
		ColorAnimation {
			target: lblMessage
			property: "color"
			from: "darkred"
			to: "white"
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	TPButton {
		id: btn1
		text: button1Text
		visible: button1Text.length > 0
		height: visible ? buttonHeight : 0
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 10

		onClicked: {
			button1Clicked();
			balloon.close();
		}

		Component.onCompleted: {
			if (button2Text.length > 0) {
				anchors.left = parent.left;
				anchors.leftMargin = (parent.width - width - btn2.width) / 3;
			}
			else
				anchors.horizontalCenter = parent.horizontalCenter;
		}
	}

	TPButton {
		id: btn2
		text: button2Text
		visible: button2Text.length > 0
		height: visible ? buttonHeight : 0
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 10

		onClicked: {
			button2Clicked();
			balloon.close();
		}

		Component.onCompleted: {
			if (button1Text.length > 0) {
				anchors.right = parent.right;
				anchors.rightMargin = (parent.width - width - btn1.width) / 3;
			}
			else
				anchors.horizontalCenter = parent.horizontalCenter;
		}
	}

	MouseArea {
		id: mouseArea
		property var prevPos
		z: 1

		onPressed: (mouse) => {
			prevPos = { x: mouse.x, y: mouse.y };
		}

		onPositionChanged: {
			const deltaX = mouseX - prevPos.x;
			if ( Math.abs(deltaX) >= 10) {
				x += deltaX;
				if (deltaX > 0)
					finalXPos = windowWidth + 300;
				else
					finalXPos = -300;
				closeTransition.enabled = false;
				alternateCloseTransition.start();
				balloon.close();
			}
			prevPos = { x: mouseX, y: mouseY };
		}

		Component.onCompleted: {
			if (button1Text.length === 0 && button2Text.length === 0)
				anchors.fill = parent;
			else {
				anchors.left = parent.left;
				anchors.right = parent.right;
				anchors.top = parent.top;
				anchors.bottom = btn1.top;
			}
		}
	}

	Timer {
		id: hideTimer
		running: false
		repeat: false
		property bool bCloseOnFinished

		onTriggered: {
			if (bCloseOnFinished)
				balloon.close();
			else
				balloon.show(startYPosition);
		}

		function delayedOpen(timeout) {
			bCloseOnFinished = false;
			interval = timeout;
			start();
			balloon.show(startYPos);
		}

		function openTimed(timeout) {
			bCloseOnFinished = true;
			interval = timeout;
			start();
			balloon.show(startYPos);
		}
	}

	function show(ypos) {
		balloon.x = (windowWidth - width)/2;
		balloon.y = finalYPos = ypos;
		if ( ypos <= windowHeight/2 )
			startYPos = -300;
		else
			startYPos = windowHeight + 300;
		balloon.open();
	}

	function showTimed(timeout, ypos) {
		startYPos = ypos;
		hideTimer.openTimed(timeout);
	}

	function showLate(timeout, ypos) {
		startYPos = ypos;
		hideTimer.delayedOpen(timeout);
	}
}
