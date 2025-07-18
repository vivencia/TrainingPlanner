import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: balloon
	keepAbove: false
	closeButtonVisible: false
	focus: false
	width: appSettings.pageWidth * 0.8
	disableMouseHandling: true

	property string message: ""
	property string title: ""
	property string button1Text: qsTr("Yes")
	property string button2Text: qsTr("No")
	property string imageSource: ""
	property string backColor: appSettings.primaryColor
	property string textColor: appSettings.fontColor
	property string subImageLabel: ""
	property bool highlightMessage: false
	property bool imageEnabled: true
	property bool movable: false
	property bool anchored: false

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

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: btnClose.left
		}

		onTextChanged: anchored = false;
	}

	TPImage {
		id: imgElement
		source: imageSource
		visible: imageSource.length > 0
		enabled: imageEnabled
		width: appSettings.itemDefaultHeight * 2
		height: width

		anchors {
			topMargin: 10
			left: parent.left
			leftMargin: 5
		}

		onSourceChanged: anchored = false;
	}

	TPLabel {
		id: lblImageSibling
		text: subImageLabel
		visible: subImageLabel.length > 0
		font: AppGlobals.smallFont

		anchors {
			left: imgElement.right
			leftMargin: -5
			bottom: imgElement.bottom
			bottomMargin: -5
		}
	}

	TPLabel {
		id: lblMessage
		text: message
		wrapMode: Text.WordWrap
		horizontalAlignment: Text.AlignJustify
		width: (imageSource.length > 0 ? balloon.width - imgElement.width : balloon.width) - 10
		visible: message.length > 0

		anchors {
			topMargin: 0
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onTextChanged: anchored = false;
	}

	RowLayout {
		spacing: 0

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
			autoSize: true
			visible: button1Text.length > 0
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
			autoSize: true
			visible: button2Text.length > 0
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

	TPMouseArea {
		movingWidget: parent
		movableWidget: parent

		onPressed: (mouse) => pressedFunction(mouse);
		onPositionChanged: (mouse) => {
			if (!balloon.movable) {
				const deltaX = mouse.x - prevPos.x;
				if (Math.abs(deltaX) >= 10) {
					x += deltaX;
					if (deltaX > 0)
						finalXPos = appSettings.pageWidth + 300;
					else
						finalXPos = -300;
					alternateCloseTransition.start();
					balloon.closePopup();
				}
				prevPos = { x: mouse.x, y: mouse.y };
			}
			else
				positionChangedFunction(mouse);
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
		let new_height = 0;
		if (title.length > 0)
			new_height = lblTitle.height + 5
		if (imageSource.length > 0)
			new_height += Math.max(imgElement.height, lblMessage.height) + 10
		else
			new_height += lblMessage.height + 10;
		if (button1Text.length > 0)
			new_height += btn1.height + 10;
		balloon.height = new_height;
		if (!anchored)
			anchorElements();
		show1(ypos);
	}

	function showTimed(timeout: int, ypos: int): void {
		hideTimer.openTimed(timeout, ypos);
	}

	function showLate(timeout: int, ypos: int): void {
		hideTimer.delayedOpen(timeout, ypos);
	}

	function anchorElements() {
		if (lblMessage.height < 50) {
			if (imageSource.length > 0) {
				imgElement.anchors.top = title.length > 0 ? lblTitle.bottom : balloon.top;
				lblMessage.anchors.verticalCenter = imgElement.verticalCenter;
			}
			else
				lblMessage.anchors.verticalCenter = balloon.verticalCenter;
		}
		else {
			lblMessage.anchors.top = title.length > 0 ? lblTitle.bottom : balloon.top;
			if (imageSource.length > 0)
				imgElement.anchors.verticalCenter = lblMessage.verticalCenter;
		}
		lblMessage.anchors.left = imageSource.length > 0 ? imgElement.right : balloon.left;
		anchored = true;
	}
}
