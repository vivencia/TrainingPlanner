import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	property string message: ""
	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string checkBoxText: ""
	property string imageSource: ""
	property string backColor: appSettings.primaryColor
	property string textColor: appSettings.fontColor
	property bool highlightMessage: false

	property int startYPosition: 0
	property int finalXPos: 0

	signal button1Clicked();
	signal button2Clicked();

	id: balloon
	bKeepAbove: true
	width: appSettings.pageWidth * 0.8

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

	TPLabel {
		id: lblTitle
		text: title
		horizontalAlignment: Text.AlignHCenter
		visible: title.length > 0
		width: parent.width - 20
		x: 10
		y: 5
	}

	TPImage {
		id: imgElement
		source: imageSource.indexOf("png") !== -1 ? appSettings.iconFolder+imageSource : imageSource
		visible: imageSource.length > 0
		width: 50
		height: 50
		layer.enabled: true
		x: 5
		y: lblTitle.visible ? (balloon.height-height)/2 : (balloon.height-height)/3
	}

	TPLabel {
		id: lblMessage
		text: message
		horizontalAlignment: Text.AlignJustify
		width: (imgElement.visible ? balloon.width - imgElement.width : balloon.width) - 25
		visible: message.length > 0
		x: imgElement.visible ? imgElement.width + 10 : 10
		y: lblTitle.visible ? lblTitle.height + 10 : imgElement.visible ? imgElement.y : 10
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
			from: appSettings.fontColor
			to: "darkred"
			duration: 700
			easing.type: Easing.InOutCubic
		}
		ColorAnimation {
			target: lblMessage
			property: "color"
			from: "darkred"
			to: appSettings.fontColor
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
					finalXPos = appSettings.pageWidth + 300;
				else
					finalXPos = -300;
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

		function delayedOpen(timeout: int) {
			bCloseOnFinished = false;
			interval = timeout;
			start();
			balloon.show(startYPos);
		}

		function openTimed(timeout: int) {
			bCloseOnFinished = true;
			interval = timeout;
			start();
			balloon.show(startYPos);
		}
	}

	function show(ypos: int) {
		balloon.height = lblTitle.height + lblMessage.height +
						(button1Text.length > 0 ? 2*btn1.buttonHeight : (button2Text.length > 0 ? 2*btn1.buttonHeight : 10));
		balloon.x = (appSettings.pageWidth - width)/2;

		if (ypos < 0)
			ypos = (appSettings.pageHeight-balloon.height)/2;

		finalYPos = ypos;
		if (ypos <= appSettings.pageHeight/2)
			startYPos = -300;
		else
			startYPos = appSettings.pageHeight + 300;
		balloon.open();
	}

	function showTimed(timeout: int, ypos: int) {
		startYPos = ypos;
		hideTimer.openTimed(timeout);
	}

	function showLate(timeout: int, ypos: int) {
		startYPos = ypos;
		hideTimer.delayedOpen(timeout);
	}
}
