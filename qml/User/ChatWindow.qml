pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Dialogs

TPPopup {
	id: _chatWindow
	closePolicy: Popup.NoAutoClose
	spacing: 0
	padding: 0
	x: normalX
	finalYPos: normalY
	width: normalWidth
	height: normalHeight
	backGroundImage: ":/images/backgrounds/backimage-chat.jpg"

//public:
	required property ChatModel chatManager

//private:
	readonly property int defaultWidth: AppSettings.pageWidth * 0.8
	readonly property bool canViewNewMessages: messagesList.vBar.position + messagesList.vBar.size >= 1

	property bool maximized: false
	property bool minimized: false
	property int normalWidth: defaultWidth
	property int normalHeight: AppSettings.pageHeight * 0.5
	property int normalX: (AppSettings.pageWidth - width) / 2
	property int normalY: (AppSettings.pageHeight - height) / 2
	property int nMedia: 0

	onOpened: {
		_chatWindow.chatManager.onChatWindowOpened();
		messagesList.positionViewAtEnd();
	}

	onActiveFocusChanged: {
		if (activeFocus)
			_chatWindow.chatManager.markAllIncomingMessagesRead();
	}

	Connections {
		target: _chatWindow.chatManager
		function onMessageReceived(): void {
			if (_chatWindow.canViewNewMessages)
				messagesList.positionViewAtEnd();
		}
	}

	TPImage {
		id: avatarImg
		source: _chatWindow.chatManager.avatarIcon
		dropShadow: false
		width: AppSettings.itemDefaultHeight
		height: width

		anchors {
			verticalCenter: _chatWindow.titleBar.verticalCenter
			left: _chatWindow.titleBar.left
			leftMargin: 2
		}
	}

	TPLabel {
		text: _chatWindow.chatManager.interlocutorName
		elide: Text.ElideRight

		anchors {
			verticalCenter: _chatWindow.titleBar.verticalCenter
			left: avatarImg.right
			leftMargin: 5
			right: btnMinimizeWindow.left
		}
	}

	TPButton {
		id: btnMaxRestoreWindow
		imageSource: _chatWindow.maximized ? "restore.png" : "maximize.png"
		hasDropShadow: false
		width: AppSettings.itemDefaultHeight
		height: width
		z: 2

		anchors {
			top: parent.top
			topMargin: 2
			right: _chatWindow.btnClose.left
			rightMargin: 2
		}

		onClicked: _chatWindow.maximizeOrRestoresWindow();
	}

	function  maximizeOrRestoresWindow(): void {
		if (minimized || maximized) {
			_chatWindow.x = normalX;
			_chatWindow.y = normalY;
			_chatWindow.width = normalWidth;
			_chatWindow.height = normalHeight;
			minimized = maximized = false;
		}
		else {
			normalX = _chatWindow.x;
			normalY = _chatWindow.y;
			normalWidth = _chatWindow.width;
			normalHeight = _chatWindow.height;
			_chatWindow.x = 0;
			_chatWindow.y = 0;
			_chatWindow.width = AppSettings.pageWidth;
			_chatWindow.height = AppSettings.pageHeight;
			maximized = true;
		}
	}

	TPButton {
		id: btnMinimizeWindow
		imageSource: "minimize.png"
		hasDropShadow: false
		enabled: !_chatWindow.minimized
		width: AppSettings.itemDefaultHeight
		height: width
		z: 2

		anchors {
			top: parent.top
			topMargin: 2
			right: btnMaxRestoreWindow.left
			rightMargin: 2
		}

		onClicked: _chatWindow.minimizeWindow();
	}

	function minimizeWindow(): void {
		normalWidth = maximized ? _chatWindow.width - 10 : _chatWindow.width;
		normalHeight = maximized ? _chatWindow.height - 10 : _chatWindow.height;
		_chatWindow.width = defaultWidth;
		_chatWindow.height = titleBar.height;
		minimized = true;
		maximized = false;
	}

	TPListView {
		id: messagesList
		contentHeight: _chatWindow.availableHeight
		contentWidth: _chatWindow.availableWidth
		spacing: 15
		clip: true
		model: _chatWindow.chatManager

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

			required property int index
			required property bool msgRead
			required property bool msgReceived
			required property bool msgSent
			required property bool msgDeleted
			required property bool msgOpenExternally
			required property bool ownMessage
			required property string msgText
			required property string msgMedia
			required property string msgSentDate
			required property string msgSentTime
			required property string msgReceivedDate
			required property string msgReceivedTime

			property int msgHeight: 10

			readonly property bool inViewport: {
				const view = messagesList.ListView.view;
				const top = view.contentY;
				const bottom = top + view.height;
				return (y + height > top) && (y < bottom)
			}

			Rectangle {
				id: messageRec
				color: messageItem.ownMessage ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2
				border.color: AppSettings.fontColor
				radius: 8
				opacity: 1 + messageItem.swipe.position
				width: mainLayout.childrenRect.width + 10
				height: mainLayout.childrenRect.height + 20
				visible: !messageItem.msgDeleted
				anchors.top: parent.top

				states: [
					State {
						when: messageItem.ownMessage

						AnchorChanges {
							target: messageRec
							anchors.right: parent.right
						}
						PropertyChanges {
							explicit: true
							anchors.rightMargin: 10
						}
					},
					State {
						when: !messageItem.ownMessage

						AnchorChanges {
							target: messageRec
							anchors.left: parent.left
						}
						PropertyChanges {
							explicit: true
							anchors.leftMargin: 10
						}
					}
				]

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
						active: messageItem.inViewport && messageItem.msgMedia.length > 0
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: active ? (messageItem.msgOpenExternally ?
															 AppSettings.itemExtraLargeHeight : _file_viewer.preferredWidth) + 10 : 0
						Layout.preferredHeight: active ? (messageItem.msgOpenExternally ?
															  AppSettings.itemExtraLargeHeight : _file_viewer.preferredHeight) + 10 : 0

						property TPFileViewer _file_viewer

						sourceComponent: TPFileViewer {
							mediaSource: messageItem.msgMedia

							onImageSizeChanged: {
								if (++_chatWindow.nMedia === _chatWindow.chatManager.nMediaMessages())
									waitTimer.start();
							}

							Component.onCompleted: mediaLoader._file_viewer = this;

							Timer {
								id: waitTimer
								interval: 100
								onTriggered: messagesList.positionViewAtEnd();
							}
						}
					}

					TPLabel {
						text: messageItem.msgText
						textFormat: Text.RichText
						singleLine: false
						visible: messageItem.msgText.length > 0
						Layout.fillWidth: true
						Layout.maximumWidth: messagesList.width * 0.9
					}

					RowLayout {
						spacing: 5
						Layout.fillWidth: true

						TPLabel {
							id: lblExtraInfo
							text: messageItem.ownMessage ? messageItem.msgSentDate + "  " + messageItem.msgSentTime :
																	messageItem.msgReceivedDate + "  " + messageItem.msgReceivedTime
							font: Qt.font({
								family: Qt.fontFamilies()[0],
								weight: Font.ExtraLight,
								italic: true,
								hintingPreference: Font.PreferFullHinting,
								pixelSize: AppSettings.smallFontSize * 0.7
							})
							Layout.fillWidth: true
						}

						Loader {
							active: messageItem.ownMessage
							Layout.preferredWidth: AppSettings.itemSmallHeight * 0.5
							Layout.preferredHeight: AppSettings.itemSmallHeight * 0.5
							Layout.alignment: Qt.AlignRight

							sourceComponent: TPImage {
								source: messageItem.msgRead ? "message-read.png" : "message-sent.png"
								visible: messageItem.msgSent
								dropShadow: false
							}
						}

						Loader {
							active: messageItem.ownMessage
							Layout.preferredWidth: AppSettings.itemSmallHeight * 0.5
							Layout.preferredHeight: AppSettings.itemSmallHeight * 0.5
							Layout.alignment: Qt.AlignRight

							sourceComponent: TPImage {
								source: messageItem.msgRead ? "message-read.png" : "message-sent.png"
								visible: messageItem.msgReceived
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

				readonly property int imageSize: height <= AppSettings.itemDefaultHeight * 4 ?
										AppSettings.itemSmallHeight * 0.9 : AppSettings.itemDefaultHeight

				Pane {
					SwipeDelegate.onClicked: _chatWindow.chatManager.removeMessage(messageItem.index, false);

					background: Rectangle {
						radius: 8
						border.color: AppSettings.fontColor
						color: "transparent"
					}
					anchors {
						top: parent.top
						left: parent.left
						right: messageItem.ownMessage ? parent.horizontalCenter : parent.right
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
					visible: messageItem.ownMessage
					SwipeDelegate.onClicked: _chatWindow.chatManager.removeMessage(messageItem.index, true);

					background: Rectangle {
						radius: 8
						border.color: AppSettings.fontColor
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
						text: qsTr("Remove as well for ") + _chatWindow.chatManager.interlocutorName
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
		visible: !_chatWindow.minimized
		padding: 5

		background: Rectangle {
			color: AppSettings.primaryColor
			border.width: 1
			border.color: AppSettings.fontColor
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
			border.color: _chatWindow.chatManager.hasUnreadMessages ? AppSettings.primaryLightColor : "transparent"
			border.width: 2
			width: AppSettings.itemDefaultHeight
			height: width
			visible: !_chatWindow.canViewNewMessages

			onClicked: {
				messagesList.vBar.setPosition(1);
				_chatWindow.chatManager.hasUnreadMessages = false;
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
					_chatWindow.sendMessage();
			}
		}

		TPButton {
			id: btnSendFile
			imageSource: "attach_"
			width: AppSettings.itemDefaultHeight
			height: width

			anchors {
				left: txtMessage.right
				leftMargin: 5
				top: txtMessage.top
			}

			onClicked: _chatWindow.sendFile();
		} //TPButton

		TPButton {
			id: btnSend
			imageSource: "send-message"
			width: AppSettings.itemDefaultHeight
			height: width
			enabled: txtMessage.text.length > 0

			anchors {
				left: txtMessage.right
				leftMargin: 5
				bottom: txtMessage.bottom
			}

			onClicked: _chatWindow.sendMessage("");
		} //TPButton

		TPImage {
			id: imgResize
			source: "resize-window.png"
			dropShadow: false
			width: AppSettings.itemSmallHeight * 0.6
			height: width
			visible: !_chatWindow.maximized && !_chatWindow.minimized
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
						_chatWindow.normalWidth += deltaX;
						const deltaY = mouse.y - startY
						_chatWindow.normalHeight += deltaY;
					}
				}
			}
		} //MouseArea
	} //Frame frmFooter

	Loader {
		id: sendFileLoader
		asynchronous: true
		active: false

		property TPFileDialog _file_dialog
		sourceComponent: TPFileDialog {
			includeAllFilesFilter: true
			onDialogClosed: (result) => {
				if (result === 0)
					_chatWindow.sendMessage(selectedFile);
				sendFileLoader.active = false;
			}
			Component.onCompleted: sendFileLoader._file_dialog = this;
		}

		onLoaded: _file_dialog.show();
	}

	function sendFile(): void {
		sendFileLoader.active = true;
	}

	function sendMessage(media_file: string): void {
		_chatWindow.chatManager.createNewMessage(txtMessage.contentsText(), media_file);
		messagesList.positionViewAtEnd();
		txtMessage.clear();
	}
}
