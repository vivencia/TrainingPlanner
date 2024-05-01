import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
	property string message: ""
	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string imageSource: ""
	property string backColor: AppSettings.primaryColor
	property string textColor: AppSettings.fontColor
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
	width: windowWidth * 0.8

	background: Rectangle {
		id: background
		color: backColor
		radius: 8
		opacity: 0.9
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
		font.pointSize: AppSettings.fontSize
	}

	Label {
		id: lblTitle
		text: title
		color: textColor
		elide: Text.ElideRight
		horizontalAlignment: Text.AlignHCenter
		font.pointSize: AppSettings.fontSizeTitle
		font.weight: Font.Black
		visible: title.length > 0
		width: parent.width - 20
		height: 30
		padding: 0
		x: 10
		y: 5
	}

	Image {
		id: imgElement
		source: imageSource
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		visible: imageSource.length > 0
		width: 50
		height: 50
		x: 5
		y: (balloon.height-height)/2
	}

	Label {
		id: lblMessage
		text: message
		color: textColor
		wrapMode: Text.WordWrap
		horizontalAlignment: Text.AlignJustify
		font.pointSize: AppSettings.fontSize
		font.weight: Font.Black
		width: (imgElement.visible ? balloon.width - imgElement.width : balloon.width) - 25
		height: Math.ceil(fontMetrics.boundingRect(message).width / balloon.width) * 30
		visible: message.length > 0
		padding: 0
		x: imgElement.visible ? imgElement.width + 10 : 10
		y: lblTitle.visible ? lblTitle.height + 10 : 10
	}

	TPButton {
		id: btn1
		text: button1Text
		visible: button1Text.length > 0
		x: button2Text.length > 0 ? (balloon.width - implicitWidth - btn2.implicitWidth)/2 : (balloon.width - implicitWidth)/2;
		y: balloon.height - buttonHeight - 5;
		z: 2

		onClicked: {
			button1Clicked();
			balloon.close();
		}
	}

	TPButton {
		id: btn2
		text: button2Text
		visible: button2Text.length > 0
		x: button1Text.length > 0 ? btn1.x + btn1.width + 5 : (balloon.width - implicitWidth)/2;
		y: balloon.height - buttonHeight - 5;
		z: 2

		onClicked: {
			button2Clicked();
			balloon.close();
		}
	}

	SequentialAnimation {
		loops: Animation.Infinite
		running: highlightMessage

		ColorAnimation {
			target: lblMessage
			property: "color"
			from: AppSettings.fontColor
			to: "darkred"
			duration: 700
			easing.type: Easing.InOutCubic
		}
		ColorAnimation {
			target: lblMessage
			property: "color"
			from: "darkred"
			to: AppSettings.fontColor
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	MouseArea {
		id: mouseArea
		property var prevPos
		z: 1
		anchors.fill: parent

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
		balloon.height = lblTitle.height + lblMessage.height + (button1Text.length > 0 ? 2*btn1.buttonHeight : (button2Text.length > 0 ? 2*btn1.buttonHeight : 10));
		balloon.x = (windowWidth - width)/2;
		balloon.y = finalYPos = ypos;
		if (ypos <= windowHeight/2)
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
