import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../Dialogs"
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
	backGroundImage: ":/images/backgrounds/backimage-chat.jpg"

	required property ChatModel chatManager

	readonly property int defaultWidth: appSettings.pageWidth * 0.8
	readonly property bool canViewNewMessages: messagesList.vBar.position + messagesList.vBar.size >= 1

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

	TPListView {
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
				width: mainLayout.childrenRect.width + 10
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

					Loader {
						id: mediaLoader
						asynchronous: true
						active: msgMedia.length > 0
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: parent.width
						Layout.preferredHeight: active ? Math.min(Math.max(appSettings.itemExtraLargeHeight, item.imgViewer.preferredHeight) + item.lblFileName.height, appSettings.pageHeight * 0.15) : 0

						sourceComponent: Item {

							readonly property TPImageViewer imgViewer: image_viewer
							readonly property TPLabel lblFileName: label_filename

							TPImageViewer {
								id: image_viewer
								previewSource: msgMediaPreview
								mediaSource: msgMedia
								openExternally: msgOpenExternally
								width: Math.min(Math.max(appSettings.itemExtraLargeHeight, preferredWidth), appSettings.pageWidth * 0.25)
								height: Math.min(Math.max(appSettings.itemExtraLargeHeight, preferredHeight), appSettings.pageHeight * 0.15)

								anchors {
									top: parent.top
									bottom: label_filename.top
									horizontalCenter: label_filename.horizontalCenter
									horizontalCenterOffset: (width - height) / 2
								}
							}

							TPLabel {
								id: label_filename
								text: msgMediaFileName
								horizontalAlignment: Text.AlignHCenter
								useBackground: true
								elide: Text.ElideRight

								anchors {
									left: parent.left
									right: parent.right
									bottom: parent.bottom
								}
							}
						}
					}

					TPLabel {
						id: lblMessageContent
						text: msgText
						textFormat: Text.RichText
						singleLine: false
						visible: msgText.length > 0
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
				messagesList.vBar.setPosition(1);
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
			id: btnSendFile
			imageSource: "attach_"
			width: appSettings.itemDefaultHeight
			height: width

			anchors {
				left: txtMessage.right
				leftMargin: 5
				top: txtMessage.top
			}

			onClicked: sendFile();
		} //TPButton

		TPButton {
			id: btnSend
			imageSource: "send-message"
			width: appSettings.itemDefaultHeight
			height: width
			enabled: txtMessage.text.length > 0

			anchors {
				left: txtMessage.right
				leftMargin: 5
				bottom: txtMessage.bottom
			}

			onClicked: sendMessage("");
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

	Loader {
		id: sendFileLoader
		asynchronous: true
		active: false

		sourceComponent: TPFileDialog {
			//includeAllFilesFilter: true
			includePDFFilter: true
			onDialogClosed: (result) => {
				if (result === 0)
					chatWindow.sendMessage(selectedFile);
				sendFileLoader.active = false;
			}
		}

		onLoaded: item.show();
	}

	function sendFile(): void {
		sendFileLoader.active = true;
	}

	function sendMessage(media_file: string): void {
		chatManager.createNewMessage(txtMessage.contentsText(), media_file);
		messagesList.positionViewAtEnd();
		txtMessage.clear();
	}
}
