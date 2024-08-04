import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"

TPPopup {
	property string message: ""
	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string checkBoxText: ""
	property string imageSource: ""
	property string backColor: AppSettings.primaryColor
	property string textColor: AppSettings.fontColor
	property bool highlightMessage: false

	property int startYPosition: 0
	property int finalXPos: 0

	signal button1Clicked();
	signal button2Clicked();

	id: balloon
	bKeepAbove: true
	width: windowWidth * 0.8

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

	FontMetrics {
		id: fontMetrics
		font.family: lblMessage.font.family
		font.pointSize: AppSettings.fontSizeText
	}

	Label {
		id: lblTitle
		text: title
		color: textColor
		elide: Text.ElideRight
		horizontalAlignment: Text.AlignHCenter
		font.pointSize: AppSettings.fontSize
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
		source: imageSource != "" ? "qrc:/images/"+AppSettings.iconFolder+imageSource : ""
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		visible: false
		width: 50
		height: 50
		layer.enabled: true
		x: 5
		y: lblTitle.visible ? (balloon.height-height)/2 : (balloon.height-height)/3
	}

	MultiEffect {
		id: imgEffects
		visible: imageSource.length > 0
		source: imgElement
		anchors.fill: imgElement
		shadowEnabled: true
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1
	}

	Label {
		id: lblMessage
		text: message
		color: textColor
		wrapMode: Text.WordWrap
		horizontalAlignment: Text.AlignJustify
		font.pointSize: AppSettings.fontSizeText
		font.weight: Font.Black
		width: (imgEffects.visible ? balloon.width - imgEffects.width : balloon.width) - 25
		height: Math.ceil(fontMetrics.boundingRect(message).width / balloon.width) * 30
		visible: message.length > 0
		padding: 0
		x: imgEffects.visible ? imgEffects.width + 10 : 10
		y: lblTitle.visible ? lblTitle.height + 10 : imgEffects.visible ? imgEffects.y : 10
	}

	TPButton {
		id: btn1
		text: button1Text
		flat: false
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
		flat: false
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
		property point prevPos
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
		balloon.height = lblTitle.height + lblMessage.height +
						(button1Text.length > 0 ? 2*btn1.buttonHeight : (button2Text.length > 0 ? 2*btn1.buttonHeight : 10));
		balloon.x = (windowWidth - width)/2;

		if (ypos < 0)
			ypos = (windowHeight-balloon.height)/2;

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
