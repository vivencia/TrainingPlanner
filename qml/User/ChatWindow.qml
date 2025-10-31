import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: chatWindow
	closePolicy: Popup.NoAutoClose
	spacing: 0
	padding: 0
	x: normalX
	y: normalY
	width: normalWidth
	height: normalHeight
	enableEffects: false

	required property ChatModel chatManager

	readonly property int defaultWidth: appSettings.pageWidth * 0.8
	property bool maximized: false
	property bool minimized: false
	property int normalWidth: defaultWidth
	property int normalHeight: appSettings.pageHeight * 0.5
	property int normalX: (appSettings.pageWidth - width) / 2
	property int normalY: (appSettings.pageHeight - height) / 2

	backgroundRec: TPBackRec {
		useImage: true
		sourceImage: ":/images/backgrounds/backimage1.jpg"
		image_size: Qt.size(width, height)
		opacity: 0.5
		radius: maximized ? 0 : 8
		layer.enabled: true
		anchors.fill: parent
	}

	TPButton {
		id: btnMaxRestoreWindow
		imageSource: maximized ? "restore.png" : "maximize.png"
		hasDropShadow: false
		width: appSettings.itemDefaultHeight
		height: width
		z: 2

		anchors {
			top: parent.top
			topMargin: 2
			right: btnClose.left
			rightMargin: 2
		}

		onClicked: maximizeOrRestoresWindow();
	}

	function  maximizeOrRestoresWindow(): void {
		if (!maximized) {
			if (!minimized) {
				normalX = chatWindow.x;
				normalY = chatWindow.y;
				normalWidth = chatWindow.width;
				normalHeight = chatWindow.height;
			}
			chatWindow.x = 0;
			chatWindow.y = 0;
			chatWindow.width = appSettings.pageWidth;
			chatWindow.height = appSettings.pageHeight;
			maximized = true;
		}
		else {
			chatWindow.x = normalX;
			chatWindow.y = normalY;
			chatWindow.width = normalWidth;
			chatWindow.height = normalHeight;
			maximized = false;
		}
		minimized = false;
	}

	TPButton {
		id: btnMinimizeWindow
		imageSource: "minimize.png"
		hasDropShadow: false
		width: appSettings.itemDefaultHeight
		height: width
		z: 2

		anchors {
			top: parent.top
			topMargin: 2
			right: btnMaxRestoreWindow.left
			rightMargin: 2
		}

		onClicked: minimizeWindow();
	}

	function minimizeWindow(): void {
		if (!maximized) {
			normalWidth = chatWindow.width;
			normalHeight = chatWindow.height;
		}
		chatWindow.width = defaultWidth;
		chatWindow.height = toolBarHeight;
		minimized = true;
		maximized = false;
	}

	ListView {
		id: messagesList
		contentHeight: availableHeight
		contentWidth: availableWidth
		spacing: 5
		model: chatManager

		anchors {
			top: btnMinimizeWindow.bottom
			topMargin: 10
			left: parent.left
			right: parent.right
			bottom: frmFooter.top
			bottomMargin: 10
		}

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: messagesList.contentHeight > messagesList.height
		}
		ScrollBar.horizontal: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: messagesList.contentWidth > messagesList.width
		}

		delegate: SwipeDelegate {
			id: messageItem
			swipe.enabled: true
			clip: true
			padding: 10
			spacing: 5
			width: messagesList.width
			height: messageRec.height

			readonly property bool senderMessage: msgSender === userModel.userId
			contentItem: Item {

				Rectangle {
					id: messageRec
					color: messageItem.senderMessage ? appSettings.listEntryColor1 : appSettings.listEntryColor2
					border.color: appSettings.fontColor
					radius: 8
					opacity: 1 + swipe.position
					width: txtMessageContent.contentWidth
					height: txtMessageContent.contentHeight + extraInfo.height
					anchors.top: parent.top

					Component.onCompleted: {
						if (messageItem.senderMessage) {
							anchors.right = parent.right;
							anchors.rightMargin = 10;
						}
						else {
							anchors.left = parent.left;
							anchors.leftMargin = 10;
						}
					}

					Text {
						id: txtMessageContent
						font: AppGlobals.regularFont
						color: appSettings.fontColor
						text: msgText
						wrapMode: Text.WordWrap

						anchors {
							top: parent.top
							left: parent.left
						}
					}

					Text {
						id: extraInfo
						font: AppGlobals.smallFont
						color: appSettings.fontColor
						text: messageItem.senderMessage ? msgSentDate + "  " + msgSentTime : msgReceivedDate + "  " + msgReceivedTime;
						width: parent.width * 0.8

						anchors {
							left: parent.left
							bottom: parent.bottom
						}
					}

					Loader {
						id: sentIconLoader
						active: messageItem.senderMessage
						asynchronous: true

						anchors {
							right: receivedIconLoader.left
							verticalCenter: extraInfo.verticalCenter
						}

						sourceComponent: TPImage {
							id: sentIcon
							source: msgRead ? "message-read.png" : "message-sent.png"
							visible: msgSent
							dropShadow: false
							width: appSettings.itemSmallHeight
							height: width
							x: 0
							y: 0
						}
					}

					Loader {
						id: receivedIconLoader
						active: messageItem.senderMessage
						asynchronous: true

						anchors {
							right: parent.right
							verticalCenter: extraInfo.verticalCenter
						}

						sourceComponent: TPImage {
							id: receivedIcon
							source: msgRead ? "message-read.png" : "message-sent.png"
							visible: msgReceived
							dropShadow: false
							width: appSettings.itemSmallHeight
							height: width
							x: 0
							y: 0
						}
					}
				}
			} //contentItem

			swipe.right: Rectangle {
				id: removeBackground
				anchors.fill: parent
				color: SwipeDelegate.pressed ? Qt.darker("tomato", 1.1) : "tomato"
				opacity: delegateItem.swipe.complete ? 0.8 : 0 - delegateItem.swipe.position

				TPImage {
					source: "remove"
					width: 50
					height: 50

					anchors {
						horizontalCenter: parent.horizontalCenter
						verticalCenter: parent.verticalCenter
					}
				}
			} //swipe.right

			swipe.onCompleted: chatManager.removeMessage(msgId);
		} //delegate
	} // messagesList

	Frame {
		id: frmFooter
		height: txtMessage.height + 12
		padding: 5

		background: Rectangle {
			border.width: 1
			border.color: appSettings.fontColor
			radius: 8
		}

		anchors {
			left: parent.left
			right: parent.right
			rightMargin: maximized ? 0 : 10
			bottom: parent.bottom
		}

		TPTextInput {
			id: txtMessage
			width: parent.width * 0.85

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}
		}

		TPButton {
			id: btnSend
			imageSource: "send-message.png"
			enabled: txtMessage.text.length > 0

			anchors {
				left: txtMessage.right
				rightMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				chatManager.newMessage(txtMessage.text);
				messagesList.positionViewAtEnd();
				//Qt.inputMethod.reset();
				txtMessage.clear();
			}
		}
	} //Frame frmFooter

	TPImage {
		source: "resize-window.png"
		dropShadow: false
		width: appSettings.itemSmalHeight * 0.6
		height: width
		visible: !maximized && !minimized

		anchors {
			right: parent.right
			bottom: parent.bottom
		}

		MouseArea {
			enabled: parent.visible
			pressAndHoldInterval: 500
			onPressAndHold: (mouse) => {
				canDrag = true;
				startX = mouse.x;
				startY = mouse.y;
			}
			onReleased: canDrag = false;
			onPositionChanged: (mouse) => {
				if (canDrag) {
					const deltaX = mouse.x - startX
					normalWidth += deltaX;
					const deltaY = mouse.y - startY
					normalHeight += deltaY;
				}
			}

			anchors.fill: parent

			property bool canDrag: false
			property int startX
			property int startY
		}
	}
}
