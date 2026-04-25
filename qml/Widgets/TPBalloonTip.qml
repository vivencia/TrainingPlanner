pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml

TPPopup {
	objectName: "TPBalloonTip"
	id: _balloon
	keepAbove: false
	showTitleBar: false
	focus: false
	open_in_window: true
	enableEffects: false
	width: AppSettings.pageWidth * 0.8
	height: mainLayout.childrenRect.height
	mouseItem: movable ? contentItem : null
	_use_burst_transition: false

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

	ColumnLayout {
		id: mainLayout
		spacing: 2

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			margins: 0
			topMargin: -10
		}

		TPLabel {
			id: lblTitle
			text: _balloon.title
			useBackground: true
			horizontalAlignment: Text.AlignHCenter
			visible: _balloon.title.length > 0
			Layout.preferredWidth: _balloon.width - 10
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
				Layout.preferredHeight: Math.max(contentHeight, imgElement.height) + 10

				/*Loader {
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
				} *///Loader
			}
		}

		RowLayout {
			visible: _balloon.button1Text.length > 0
			spacing: empty_space
			Layout.fillWidth: true
			Layout.leftMargin: empty_space
			Layout.preferredHeight: AppSettings.itemDefaultHeight

			readonly property int empty_space: (_balloon.width - btn1.width - btn2.width) / 3

			TPButton {
				id: btn1
				text: _balloon.button1Text
				visible: _balloon.button1Text.length > 0
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: _balloon.availableWidth - btn2.width - 10

				onClicked: {
					_balloon.button1Clicked();
					_balloon.closePopup();
				}
			}

			TPButton {
				id: btn2
				text: _balloon.button2Text
				visible: _balloon.button2Text.length > 0
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: Math.min(preferredWidth, _balloon.availableWidth / 2 - 10)

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

		onTriggered: {
			if (bCloseOnFinished)
				_balloon.closePopup();
			else
				_balloon.tpOpen();
		}

		function delayedOpen(timeout: int): void {
			bCloseOnFinished = false;
			interval = timeout;
			start();
		}

		function openTimed(timeout: int): void {
			bCloseOnFinished = true;
			interval = timeout;
			start();
			_balloon.tpOpen();
		}
	}

	function showTimed(timeout: int): void {
		hideTimer.openTimed(timeout);
	}

	function showLate(timeout: int): void {
		hideTimer.delayedOpen(timeout);
	}
}
