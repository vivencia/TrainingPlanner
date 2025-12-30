import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: balloon
	keepAbove: false
	closeButtonVisible: false
	showTitleBar: title.length > 0
	focus: false
	width: appSettings.pageWidth * 0.8
	height: mainLayout.childrenRect.height * 1.1
	disableMouseHandling: !movable

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

	ColumnLayout {
		id: mainLayout
		spacing: 10
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			margins: 5
		}

		TPLabel {
			id: lblTitle
			text: title
			horizontalAlignment: Text.AlignHCenter
			visible: title.length > 0
			Layout.maximumWidth: parent.width - titleBarHeight
		}

		RowLayout {
			Layout.fillWidth: true

			TPImage {
				id: imgElement
				source: imageSource
				visible: imageSource.length > 0
				enabled: imageEnabled
				Layout.preferredWidth: appSettings.itemExtraLargeHeight
				Layout.preferredHeight: appSettings.itemExtraLargeHeight
				Layout.alignment: Qt.AlignVCenter

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
			}

			TPLabel {
				id: lblMessage
				text: message
				singleLine: false
				horizontalAlignment: Text.AlignHCenter
				visible: message.length > 0
				Layout.fillWidth: true
				Layout.maximumHeight: contentHeight
			}
		}

		Row {
			visible: button1Text.length > 0 || button2Text.length > 0
			Layout.fillWidth: true
			Layout.leftMargin: empty_space
			spacing: empty_space
			height: Math.max(Math.max(appSettings.itemDefaultHeight, btn1.height), btn2.height);

			readonly property int empty_space: (balloon.width - btn1.width - btn2.width) / 3
			TPButton {
				id: btn1
				text: button1Text
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

	Loader {
		active: !movable
		asynchronous: true
		anchors.fill: parent

		sourceComponent:  TPMouseArea {
			movingWidget: parent
			movableWidget: parent

			property point prevPos

			onPressed: (mouse) => prevPos = { x: mouse.x, y: mouse.y };
			onPositionChanged: (mouse) => {
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
		}
	} //Loadeer

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
		show1(ypos);
	}

	function showTimed(timeout: int, ypos: int): void {
		hideTimer.openTimed(timeout, ypos);
	}

	function showLate(timeout: int, ypos: int): void {
		hideTimer.delayedOpen(timeout, ypos);
	}
}
