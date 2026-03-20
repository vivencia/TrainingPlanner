pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml

TPPopup {
	id: _balloon
	keepAbove: false
	closeButtonVisible: false
	showTitleBar: false
	focus: false
	width: AppSettings.pageWidth * 0.8
	height: mainLayout.childrenRect.height * 1.1
	disableMouseHandling: !movable

//public:
	property string message: ""
	property string title: ""
	property string button1Text: qsTr("Yes")
	property string button2Text: qsTr("No")
	property string imageSource: ""
	property string backColor: AppSettings.primaryColor
	property string textColor: AppSettings.fontColor
	property string subImageLabel: ""
	property bool highlightMessage: false
	property bool imageEnabled: true
	property bool movable: false

	signal button1Clicked();
	signal button2Clicked();

//private:
	property int finalXPos: 0

	NumberAnimation {
		id: alternateCloseTransition
		target: _balloon
		alwaysRunToEnd: true
		running: false
		property: "x"
		from: _balloon.x
		to: _balloon.finalXPos
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
			text: _balloon.title
			useBackground: true
			horizontalAlignment: Text.AlignHCenter
			visible: _balloon.title.length > 0
			Layout.fillWidth: true
		}

		RowLayout {
			Layout.fillWidth: true

			TPImage {
				id: imgElement
				source: _balloon.imageSource
				visible: _balloon.imageSource.length > 0
				enabled: _balloon.imageEnabled
				Layout.preferredWidth: AppSettings.itemExtraLargeHeight
				Layout.preferredHeight: AppSettings.itemExtraLargeHeight
				Layout.alignment: Qt.AlignVCenter

				TPLabel {
					id: lblImageSibling
					text: _balloon.subImageLabel
					visible: _balloon.subImageLabel.length > 0
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
				text: _balloon.message
				singleLine: false
				horizontalAlignment: Text.AlignHCenter
				visible: _balloon.message.length > 0
				Layout.fillWidth: true
				Layout.maximumHeight: contentHeight

				Loader {
					active: !_balloon.movable
					asynchronous: true
					anchors.fill: parent

					sourceComponent:  TPMouseArea {
						movingWidget: parent
						movableWidget: _balloon

						property point prevPos

						onPressed: (mouse) => prevPos = { x: mouse.x, y: mouse.y };
						onPositionChanged: (mouse) => {
							const deltaX = mouse.x - prevPos.x;
							if (Math.abs(deltaX) >= 10) {
								x += deltaX;
								if (deltaX > 0)
									_balloon.finalXPos = AppSettings.pageWidth + 300;
								else
									_balloon.finalXPos = -300;
								alternateCloseTransition.start();
								_balloon.closePopup();
							}
							prevPos = { x: mouse.x, y: mouse.y };
						}
					}
				} //Loader
			}
		}

		Row {
			visible: _balloon.button1Text.length > 0 || _balloon.button2Text.length > 0
			spacing: empty_space
			Layout.fillWidth: true
			Layout.leftMargin: empty_space
			Layout.preferredHeight: Math.max(Math.max(AppSettings.itemDefaultHeight, btn1.height), btn2.height);

			readonly property int empty_space: (_balloon.width - btn1.width - btn2.width) / 3
			TPButton {
				id: btn1
				text: _balloon.button1Text
				autoSize: true
				visible: _balloon.button1Text.length > 0
				Layout.alignment: Qt.AlignCenter

				onClicked: {
					_balloon.button1Clicked();
					_balloon.closePopup();
				}
			}

			TPButton {
				id: btn2
				text: _balloon.button2Text
				autoSize: true
				visible: _balloon.button2Text.length > 0
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: _balloon.availableWidth - btn1.width - 10

				onClicked: {
					_balloon.button2Clicked();
					_balloon.closePopup();
				}
			}
		}
	} // ColumnLayout

	SequentialAnimation {
		loops: Animation.Infinite
		running: _balloon.highlightMessage

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

	Timer {
		id: hideTimer
		running: false
		repeat: false

		property bool bCloseOnFinished
		property int ypos

		onTriggered: {
			if (bCloseOnFinished)
				_balloon.closePopup();
			else
				_balloon.show(ypos);
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
			_balloon.show(ypos);
		}
	}

	function show(ypos: int): void {
		_balloon.show1(ypos);
	}

	function showTimed(timeout: int, ypos: int): void {
		hideTimer.openTimed(timeout, ypos);
	}

	function showLate(timeout: int, ypos: int): void {
		hideTimer.delayedOpen(timeout, ypos);
	}
}
