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
	finalYPos: normalY
	width: normalWidth
	height: normalHeight
	backgroundRec: backRec

	required property ChatModel chatManager

	readonly property int defaultWidth: appSettings.pageWidth * 0.8
	readonly property bool canViewNewMessages: mBar.position + mBar.size >= 1

	property bool maximized: false
	property bool minimized: false
	property int normalWidth: defaultWidth
	property int normalHeight: appSettings.pageHeight * 0.5
	property int normalX: (appSettings.pageWidth - width) / 2
	property int normalY: (appSettings.pageHeight - height) / 2

	onOpened: {
		chatManager.onChatWindowOpened();
		messagesList.positionViewAtEnd();
	}

	onActiveFocusChanged: {
		if (activeFocus)
			chatManager.markAllIncomingMessagesRead();
	}

	Connections {
		target: chatManager
		function onMessageReceived(): void {
			if (canViewNewMessages)
				messagesList.positionViewAtEnd();
		}
	}

	TPBackRec {
		id: backRec
		useImage: true
		scaleImageToControlSize: false
		sourceImage: ":/images/backgrounds/backimage-chat.jpg"
		radius: 8
		opacity: 0.8
		anchors.fill: parent
	}

	TPImage {
		id: avatarImg
		source: chatManager.avatarIcon
		dropShadow: false
		width: appSettings.itemDefaultHeight
		height: width

		anchors {
			verticalCenter: titleBar.verticalCenter
			left: titleBar.left
			leftMargin: 2
		}
	}

	TPLabel {
		text: chatManager.interlocutorName
		elide: Text.ElideRight

		anchors {
			verticalCenter: titleBar.verticalCenter
			left: avatarImg.right
			leftMargin: 5
			right: btnMinimizeWindow.left
		}
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
		if (minimized || maximized) {
			chatWindow.x = normalX;
			chatWindow.y = normalY;
			chatWindow.width = normalWidth;
			chatWindow.height = normalHeight;
			minimized = maximized = false;
		}
		else {
			normalX = chatWindow.x;
			normalY = chatWindow.y;
			normalWidth = chatWindow.width;
			normalHeight = chatWindow.height;
			chatWindow.x = 0;
			chatWindow.y = 0;
			chatWindow.width = appSettings.pageWidth;
			chatWindow.height = appSettings.pageHeight;
			maximized = true;
		}
	}

	TPButton {
		id: btnMinimizeWindow
		imageSource: "minimize.png"
		hasDropShadow: false
		enabled: !minimized
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
		normalWidth = maximized ? chatWindow.width - 10 : chatWindow.width;
		normalHeight = maximized ? chatWindow.height - 10 : chatWindow.height;
		chatWindow.width = defaultWidth;
		chatWindow.height = titleBar.height;
		minimized = true;
		maximized = false;
	}

	ListView {
		id: messagesList
		contentHeight: availableHeight
		contentWidth: availableWidth
		spacing: 15
		clip: true
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
			id: mBar
			policy: ScrollBar.AsNeeded
			visible: messagesList.contentHeight > messagesList.height
		}
		ScrollBar.horizontal: ScrollBar {
			policy: ScrollBar.AlwaysOff
		}

		delegate: SwipeDelegate {
			id: messageItem
			swipe.enabled: !msgDeleted
			clip: true
			padding: 0
			spacing: 0
			width: messagesList.width
			height: !msgDeleted ? messageRec.height : 0

			property int msgHeight: 10

			Rectangle {
				id: messageRec
				color: ownMessage ? appSettings.listEntryColor1 : appSettings.listEntryColor2
				border.color: appSettings.fontColor
				radius: 8
				opacity: 1 + swipe.position
				width: mainLayout.childrenRect.width + 20
				height: mainLayout.childrenRect.height + 20
				visible: !msgDeleted
				anchors.top: parent.top

				Component.onCompleted: {
					if (ownMessage) {
						anchors.right = parent.right;
						anchors.rightMargin = 10;
					}
					else {
						anchors.left = parent.left;
						anchors.leftMargin = 10;
					}
				}

				ColumnLayout {
					id: mainLayout
					spacing: 2
					anchors {
						left: parent.left
						top: parent.top
						margins: 5
					}

					TPLabel {
						id: lblMessageContent
						text: msgText
						textFormat: Text.RichText
						singleLine: false
						Layout.fillWidth: true
						Layout.maximumWidth: messagesList.width * 0.9
					}

					RowLayout {
						spacing: 5
						Layout.fillWidth: true

						TPLabel {
							id: lblExtraInfo
							text: ownMessage ? msgSentDate + "  " + msgSentTime : msgReceivedDate + "  " + msgReceivedTime
							font: Qt.font({
								family: Qt.fontFamilies()[0],
								weight: Font.ExtraLight,
								italic: true,
								styleStrategy: Font.PreferAntialias,
								hintingPreference: Font.PreferFullHinting,
								pixelSize: appSettings.smallFontSize * 0.7
							})
							Layout.alignment: Qt.AlignLeft
							Layout.fillWidth: true
						}

						Loader {
							active: ownMessage
							width: appSettings.itemSmallHeight * 0.5
							height: width
							Layout.alignment: Qt.AlignRight

							sourceComponent: TPImage {
								source: msgRead ? "message-read.png" : "message-sent.png"
								visible: msgSent
								dropShadow: false
							}
						}

						Loader {
							active: ownMessage
							width: appSettings.itemSmallHeight * 0.5
							height: width
							Layout.alignment: Qt.AlignRight

							sourceComponent: TPImage {
								source: msgRead ? "message-read.png" : "message-sent.png"
								visible: msgReceived
								dropShadow: false
							}
						}
					} //RowLayout
				}//ColumnLayout
			} //Rectangle messageRec

			background: Rectangle {
				color: "transparent"
			}

			swipe.right: Rectangle {
				id: removeBackground
				color: SwipeDelegate.pressed ? Qt.darker("tomato", 1.1) : "tomato"
				opacity: messageItem.swipe.complete ? 0.9 : 0 - messageItem.swipe.position
				radius: 8
				anchors.fill: messageRec

				readonly property int imageSize: height <= appSettings.itemDefaultHeight * 4 ?
										appSettings.itemSmallHeight * 0.9 : appSettings.itemDefaultHeight

				Pane {
					SwipeDelegate.onClicked: chatManager.removeMessage(index, false);

					background: Rectangle {
						radius: 8
						border.color: appSettings.fontColor
						color: "transparent"
					}
					anchors {
						top: parent.top
						left: parent.left
						right: ownMessage ? parent.horizontalCenter : parent.right
						bottom: parent.bottom
					}

					TPImage {
						id: delImage1
						source: "remove"
						width: removeBackground.imageSize
						height: width

						anchors {
							horizontalCenter: parent.horizontalCenter
							verticalCenter: parent.verticalCenter
							verticalCenterOffset: - height / 2
						}
					}

					TPLabel {
						text: qsTr("Remove for myself only")
						color: "white"
						singleLine: false
						width: parent.width - 5

						anchors {
							horizontalCenter: parent.horizontalCenter
							top: delImage1.bottom
							bottom: parent.bottom
						}
					}
				}

				Pane {
					visible: ownMessage
					SwipeDelegate.onClicked: chatManager.removeMessage(index, true);

					background: Rectangle {
						radius: 8
						border.color: appSettings.fontColor
						color: "transparent"
					}

					anchors {
						top: parent.top
						left: parent.horizontalCenter
						right: parent.right
						bottom: parent.bottom
					}

					TPImage {
						id: delImage2
						source: "remove"
						width: removeBackground.imageSize
						height: width

						anchors {
							horizontalCenter: parent.horizontalCenter
							horizontalCenterOffset: - width / 2
							verticalCenter: parent.verticalCenter
							verticalCenterOffset: - height / 2
						}
					}
					TPImage {
						id: delImage3
						source: "remove"
						width: removeBackground.imageSize
						height: width

						anchors {
							left: delImage2.right
							leftMargin: - width / 2
							top: delImage2.top
							topMargin: width / 3
						}
					}

					TPLabel {
						text: qsTr("Remove as well for ") + chatManager.interlocutorName
						color: "white"
						singleLine: false
						width: parent.width - 5

						anchors {
							horizontalCenter: parent.horizontalCenter
							top: delImage3.bottom
							bottom: parent.bottom
						}
					}
				}
			} //swipe.right
		} //delegate
	} // messagesList

	Frame {
		id: frmFooter
		height: txtMessage.height + 12
		visible: !minimized
		padding: 5

		background: Rectangle {
			color: appSettings.primaryColor
			border.width: 1
			border.color: appSettings.fontColor
			radius: 8
		}

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		TPButton {
			id: btnScrollDown
			imageSource: "downward"
			hasDropShadow: false
			border.color: chatManager.hasUnreadMessages ? appSettings.primaryLightColor : "transparent"
			border.width: 2
			width: appSettings.itemDefaultHeight
			height: width
			visible: !canViewNewMessages

			onClicked: {
				mBar.setPosition(1);
				chatManager.hasUnreadMessages = false;
			}

			anchors {
				bottom: parent.top
				bottomMargin: 20
				right: parent.right
				rightMargin: 10
			}
		}

		TPMultiLineEdit {
			id: txtMessage
			width: parent.width * 0.88

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}

			onEnterOrReturnKeyPressed: (mod_key) => {
				if (mod_key === Qt.Key_Control)
					sendMessage();
			}
		}

		TPButton {
			id: btnSend
			imageSource: "send-message"
			width: appSettings.itemDefaultHeight
			height: width
			enabled: txtMessage.text.length > 0

			anchors {
				left: txtMessage.right
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: sendMessage();
		} //TPButton

		TPImage {
			id: imgResize
			source: "resize-window.png"
			dropShadow: false
			width: appSettings.itemSmallHeight * 0.6
			height: width
			visible: !maximized && !minimized
			z: 1

			anchors {
				right: parent.right
				rightMargin: -5
				bottom: parent.bottom
				bottomMargin: -10
			}

			MouseArea {
				enabled: parent.visible
				anchors.fill: parent

				property bool canDrag: false
				property int startX
				property int startY

				onPressed: (mouse) => {
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
			}
		} //MouseArea
	} //Frame frmFooter

	function sendMessage() {
		chatManager.createNewMessage(txtMessage.messageText());
		messagesList.positionViewAtEnd();
		txtMessage.clear();
	}
}
