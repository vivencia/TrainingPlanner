import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: balloon
	bKeepAbove: true
	closeButtonVisible: false
	width: appSettings.pageWidth * 0.8

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
		width: parent.width - (closeButtonVisible ? 20 : 10)
		x: closeButtonVisible ? 5 : 10
		y: closeButtonVisible ? 10 : 5
	}

	TPImage {
		id: imgElement
		source: imageSource
		visible: imageSource.length > 0
		width: 50
		height: 50
		layer.enabled: true
		x: 5
		y: title.length > 0 ? (balloon.height-height)/2 : (balloon.height-height)/3
	}

	TPLabel {
		id: lblMessage
		text: message
		wrapMode: Text.WordWrap
		horizontalAlignment: Text.AlignJustify
		width: (imageSource.length > 0 ? balloon.width - imgElement.width : balloon.width) - 10
		visible: message.length > 0
		x: imageSource.length > 0 ? imgElement.width + 5 : 5
		y: title.length > 0 ? lblTitle.y + lblTitle.height + 10 : imageSource.length > 0 ? imgElement.y : 10
	}

	RowLayout {
		spacing: 0
		z: 2

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
			bottomMargin: 5
		}

		TPButton {
			id: btn1
			text: button1Text
			flat: false
			autoResize: true
			visible: button1Text.length > 0
			z: 2
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				button1Clicked();
				balloon.close();
			}
		}

		TPButton {
			id: btn2
			text: button2Text
			flat: false
			autoResize: true
			visible: button2Text.length > 0
			z: 2
			Layout.alignment: Qt.AlignCenter
			Layout.maximumWidth: availableWidth - btn1.width - 10

			onClicked: {
				button2Clicked();
				balloon.close();
			}
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

		function delayedOpen(timeout: int): void {
			bCloseOnFinished = false;
			interval = timeout;
			start();
			balloon.show(startYPos);
		}

		function openTimed(timeout: int): void {
			bCloseOnFinished = true;
			interval = timeout;
			start();
			balloon.show(startYPos);
		}
	}

	function show(ypos: int): void {
		balloon.height = lblTitle.height + Math.max(imgElement.height, lblMessage.height) +
						(button1Text.length > 0 ? 2*btn1.height : 0)
		show1(ypos);
	}

	function showTimed(timeout: int, ypos: int): void {
		startYPos = ypos;
		hideTimer.openTimed(timeout);
	}

	function showLate(timeout: int, ypos: int): void {
		startYPos = ypos;
		hideTimer.delayedOpen(timeout);
	}
}
