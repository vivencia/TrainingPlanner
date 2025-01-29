import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: balloon
	keepAbove: true
	closeButtonVisible: false
	focus: false
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
	property bool closable: true
	property bool imageEnabled: true

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
		heightAvailable: 30
		visible: title.length > 0
		width: parent.width - (closeButtonVisible ? 20 : 10)

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: btnClose.left
		}
	}

	TPImage {
		id: imgElement
		source: imageSource
		visible: imageSource.length > 0
		enabled: imageEnabled
		width: 50
		height: 50

		anchors {
			top: title.length > 0 ? lblTitle.bottom : parent.top
			topMargin: 20
			left: parent.left
			leftMargin: 5
		}
	}

	TPLabel {
		id: lblMessage
		text: message
		wrapMode: Text.WordWrap
		heightAvailable: 50
		horizontalAlignment: Text.AlignJustify
		width: (imageSource.length > 0 ? balloon.width - imgElement.width : balloon.width) - 10
		visible: message.length > 0

		anchors {
			top: title.length > 0 ? lblTitle.bottom : parent.top
			topMargin: 20
			left: imageSource.length > 0 ? imgElement.right : parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		Component.onCompleted: anchorElements();
		onSizeChanged: anchorElements();

		function anchorElements() {
			if (lblMessage.height < 50) {
				if (imageSource.length > 0) {
					imgElement.anchors.top = title.length > 0 ? lblTitle.bottom : parent.top;
					imgElement.anchors.topMargin = 5;
					anchors.verticalCenter = imgElement.verticalCenter;
				}
				else
					anchors.verticalCenter = parent.verticalCenter;
			}
			else {
				anchors.top = title.length > 0 ? lblTitle.bottom : parent.top;
				anchors.topMargin = 10;
				if (imageSource.length > 0)
					imgElement.anchors.verticalCenter = verticalCenter;
			}
		}
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
				balloon.closePopup();
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
				balloon.closePopup();
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
		enabled: closable
		z: 1
		anchors.fill: parent

		property point prevPos

		onPressed: (mouse) => prevPos = { x: mouse.x, y: mouse.y };

		onPositionChanged: {
			const deltaX = mouseX - prevPos.x;
			if (Math.abs(deltaX) >= 10) {
				x += deltaX;
				if (deltaX > 0)
					finalXPos = appSettings.pageWidth + 300;
				else
					finalXPos = -300;
				alternateCloseTransition.start();
				balloon.closePopup();
			}
			prevPos = { x: mouseX, y: mouseY };
		}
	}

	Timer {
		id: hideTimer
		running: false
		repeat: false

		property bool bCloseOnFinished
		property int ypos

		onTriggered: {
			if (bCloseOnFinished)
				balloon.closePopup();
			else
				balloon.show(ypos);
		}

		function delayedOpen(timeout: int, ypos: int): void {
			bCloseOnFinished = false;
			interval = timeout;
			hideTimer.ypos = ypos;
			start();
		}

		function openTimed(timeout: int, ypos: int): void {
			bCloseOnFinished = true;
			interval = timeout;
			start();
			balloon.show(ypos);
		}
	}

	function show(ypos: int): void {
		balloon.height = (title.length > 0 ? lblTitle.height + 5 : 0) + (imageSource.length > 0 ? Math.max(imgElement.height, lblMessage.height) : lblMessage.height) +
					(button1Text.length > 0 ? btn1.height + 10 : 0) + 5;
		show1(ypos);
	}

	function showTimed(timeout: int, ypos: int): void {
		hideTimer.openTimed(timeout, ypos);
	}

	function showLate(timeout: int, ypos: int): void {
		hideTimer.delayedOpen(timeout, ypos);
	}
}
