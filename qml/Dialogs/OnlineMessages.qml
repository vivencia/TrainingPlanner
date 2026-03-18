pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.User

TPPopup {
	id: onlineMsgsDlg
	keepAbove: true
	showTitleBar: false
	closeButtonVisible: false
	disableMouseHandling: true
	x: AppSettings.pageWidth - 80
	finalYPos: 180
	width: mainIcon.width
	height: mainIcon.height
	backgroundRec.visible: fullDialogVisible
	backGroundImage: ":/images/backgrounds/backimage-messages.jpg"

	property bool fullDialogVisible: false
	property int mainIconUserDefinedX: x
	property int mainIconUserDefinedY: y
	readonly property int dlgMaxWidth: AppSettings.pageWidth * 0.8
	readonly property int maxHeight: AppSettings.pageHeight * 0.5

	ParallelAnimation
	{
		id: shrink
		alwaysRunToEnd: true

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "width"
			to: mainIcon.width
			duration: 200
			easing.type: Easing.OutQuad
		}

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "height"
			to: mainIcon.height
			duration: 200
			easing.type: Easing.OutQuad
		}

		onFinished: {
			onlineMsgsDlg.x = onlineMsgsDlg.mainIconUserDefinedX;
			onlineMsgsDlg.y = onlineMsgsDlg.mainIconUserDefinedY;
			onlineMsgsDlg.fullDialogVisible = false;
		}
	}
	ParallelAnimation
	{
		id: expand
		alwaysRunToEnd: true

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "width"
			to: onlineMsgsDlg.dlgMaxWidth
			duration: 200
			easing.type: Easing.OutQuad
		}

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "height"
			to: topBar.height + mainLayout.height
			duration: 200
			easing.type: Easing.OutQuad
		}

		onFinished: {
			if ((onlineMsgsDlg.x + onlineMsgsDlg.width) > AppSettings.pageWidth)
				onlineMsgsDlg.x = AppSettings.pageWidth - onlineMsgsDlg.width - 10;
			onlineMsgsDlg.fullDialogVisible = true;
		}
	}

	TPImage {
		id: mainIcon
		source: "messages"
		width: AppSettings.itemExtraLargeHeight
		height: width
		visible: !onlineMsgsDlg.fullDialogVisible
		anchors {
			verticalCenter: parent.verticalCenter
			horizontalCenter: parent.horizontalCenter
		}

		TPMouseArea {
			movingWidget: mainIcon
			movableWidget: onlineMsgsDlg
			enabled: !onlineMsgsDlg.fullDialogVisible

			onMouseClicked: {
				onlineMsgsDlg.mainIconUserDefinedX = onlineMsgsDlg.x;
				onlineMsgsDlg.mainIconUserDefinedY = onlineMsgsDlg.y;
				expand.start();
			}
		}
	}

	TPLabel {
		id: topBar
		text: qsTr("Messages")
		visible: onlineMsgsDlg.fullDialogVisible
		height: AppSettings.itemLargeHeight
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter;
			horizontalCenterOffset: 0 - smallIcon.width/2
		}

		TPImage {
			id: smallIcon
			source: "messages"
			dropShadow: false
			width: AppSettings.itemDefaultHeight
			height: width

			anchors {
				left: topBar.right
				verticalCenter: topBar.verticalCenter;
			}
		}

		TPMouseArea {
			movingWidget: parent
			movableWidget: onlineMsgsDlg

			onMouseClicked: {
				if (ItemManager.appPagesManager.isPopupAboveAllOthers(onlineMsgsDlg)) {
					onlineMsgsDlg.mainIconUserDefinedY = onlineMsgsDlg.y;
					shrink.start();
				}
				else
					ItemManager.appPagesManager.raisePopup(onlineMsgsDlg);
			}
		}
	}

	StackLayout {
		id: mainLayout
		visible: onlineMsgsDlg.fullDialogVisible
		currentIndex: AppMessages ? AppMessages.count > 0 ? 1 : 0 : 0
		height: childrenRect.height

		anchors {
			top: topBar.bottom
			left: parent.left
			right: parent.right
		}

		TPLabel {
			text: qsTr("No messages")
			useBackground: true
			horizontalAlignment: Qt.AlignHCenter
			font: AppGlobals.largeFont
			Layout.preferredHeight: onlineMsgsDlg.maxHeight / 2
			Layout.fillWidth: true
		}

		TPListView {
			id: messagesList
			model: AppMessages
			Layout.fillWidth: true
			Layout.fillHeight: true

			property int messagesHeight: 0

			delegate: SwipeDelegate {
				id: delegateItem
				swipe.enabled: messagesList.model.sticky
				clip: true
				padding: 0
				spacing: 5
				width: messagesList.width

				required property int index
				property bool showActions: false

				function setShowActions(show: bool) {
					if (show !== showActions) {
						messagesList.messagesHeight -= actionsLoader.actionsLayout.childrenRect.height + lblMessage.height;
						showActions = show;
						messagesList.messagesHeight += actionsLoader.actionsLayout.childrenRect.height + lblMessage.height;
					}
				}

				background: Rectangle {
					opacity: 0.8
					color: delegateItem.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2
				}

				contentItem: Column {
					id: messageLayout
					spacing: 5
					padding: 5
					opacity: 1 + delegateItem.swipe.position

					MouseArea {
						width:  childrenRect.width
						height: childrenRect.height
						onClicked: delegateItem.setShowActions(!delegateItem.showActions);

						RowLayout {
							id: messageTextLayout
							height: 20
							Layout.fillWidth: true
							Layout.leftMargin: 10
							Layout.rightMargin: 10

							TPImage {
								source: messagesList.model.msgicon
								imageSizeFollowControlSize: true
								keepAspectRatio: true
								fullWindowView: false
								dropShadow: false
								Layout.preferredWidth: AppSettings.itemSmallHeight
								Layout.preferredHeight: AppSettings.itemSmallHeight
								visible: messagesList.model.msgicon.length > 0
							}

							TPLabel {
								text: messagesList.model.msgdate + "  " + messagesList.model.msgtime
								font: AppGlobals.smallFont
								Layout.preferredHeight: AppSettings.itemSmallHeight
								Layout.leftMargin: AppSettings.itemSmallHeight
							}

							Item {
								Layout.preferredWidth: AppSettings.itemSmallHeight
								Layout.preferredHeight: AppSettings.itemSmallHeight
								visible: messagesList.model.extraInfo.length > 0

								TPImage {
									id: extraInfoImg
									source: messagesList.model.extraInfoIcon
									anchors.fill: parent
								}
								TPLabel {
									text: messagesList.model.extraInfo
									minimumPixelSize: AppSettings.smallFontSize * 0.7
									z: 1
									width: parent.width * 0.5
									height: parent.height * 0.8
									anchors.centerIn: extraInfoImg
								}
							}

							TPImage {
								id: btnFoldIcon
								source: delegateItem.showActions ? "fold-up.png" : "fold-down.png"
								dropShadow: false
								Layout.preferredWidth: 18
								Layout.preferredHeight: 18
								Layout.leftMargin: 30
							}
						}
					}

					TPLabel {
						id: lblMessage
						text: messagesList.model.labelText
						elide: delegateItem.showActions ? Text.ElideNone : Text.ElideRight
						wrapMode: delegateItem.showActions ? Text.WordWrap : Text.NoWrap
						singleLine: !delegateItem.showActions
						width: onlineMsgsDlg.dlgMaxWidth - 10
						height: delegateItem.showActions ? contentHeight : AppSettings.itemDefaultHeight
						Layout.leftMargin: 20

						MouseArea {
							anchors.fill: parent
							onClicked: {
								if (messagesList.model.hasActions)
									delegateItem.setShowActions(!delegateItem.showActions);
								else
									AppMessages.itemClicked(delegateItem.index);
							}
						}
					}

					Loader {
						asynchronous: true
						active: messagesList.model.filename.length > 0
						Layout.maximumWidth: onlineMsgsDlg.dlgMaxWidth
						Layout.minimumWidth: onlineMsgsDlg.dlgMaxWidth

						sourceComponent: TPFileViewer {
							mediaSource: messagesList.model.filename
							onRemovalRequested: AppMessages.removeMessage(messagesList.model.msgid);
						}
					}

					Loader {
						id: actionsLoader
						asynchronous: true
						active: messagesList.model.actions.length > 0
						Layout.maximumWidth: onlineMsgsDlg.dlgMaxWidth
						Layout.minimumWidth: onlineMsgsDlg.dlgMaxWidth

						property GridLayout actionsLayout

						sourceComponent: GridLayout {
							id: _layout
							columns: 2
							visible: delegateItem.showActions
							columnSpacing: 2
							rowSpacing: 5

							readonly property int maxButtonWidth: (onlineMsgsDlg.dlgMaxWidth - 25)/3
							readonly property int msgIndex: delegateItem.index

							Repeater {
								id: actionsRepeater
								model: messagesList.model.actions

								delegate: TPButton {
									text: messagesList.model.actions[index]
									width: constrainSize ? _layout.maxButtonWidth : preferredWidth
									autoSize: !constrainSize
									rounded: false
									Layout.alignment: Qt.AlignCenter
									onClicked: AppMessages.execAction(_layout.msgIndex, index);

									required property int index
									property bool constrainSize: false
									property int row: 0
									property int col: 0
								}

								//items are added in reverse order(from last to first) from TPMessages::insertAction call order
								onItemAdded: (index, item) => {
									if (index === 0) {
										const n_items = actionsRepeater.model.count;
										let row_width = 0, row  = 0, col = 0;
										for (let i = 0; i < n_items; ++i) {
											row_width += itemAt(i).width;
											if (col === 0 || (row_width < onlineMsgsDlg.dlgMaxWidth - 5)) {
												itemAt(i).Layout.column = col;
												itemAt(1).Layout.row = row;
												col++
											}
											else {
												if (itemAt(i).width >= itemAt(i-1).width)
													itemAt(i).constrainSize = true;
												else
													itemAt(i-1).constrainSize = true;
												if (itemAt(i).width + itemAt(i-1).width >= onlineMsgsDlg.dlgMaxWidth - 5) {
													itemAt(i).Layout.column = col = 0;
													itemAt(i).Layout.row = ++row;
													itemAt(i-1).constrainSize = false;
												}
												else {
													itemAt(i).Layout.column = col;
													itemAt(1).Layout.row = row;
													col++
												}
											}
										}
										messagesList.messagesHeight +=
														_layout.childrenRect.height + lblMessage.height + messageTextLayout.height;
									}
								} //onItemAdded
							} //Repeater

							Component.onCompleted: actionsLoader.actionsLayout = this;
						} //sourceComponent: GridLayout
					} //Loader
				} //contentItem: Column

				swipe.right: Rectangle {
					id: removeBackground
					anchors.fill: parent
					color: SwipeDelegate.pressed ? Qt.darker("tomato", 1.1) : "tomato"
					opacity: delegateItem.swipe.complete ? 0.8 : 0-delegateItem.swipe.position

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
				swipe.onCompleted: AppMessages.removeMessage(index);
			} //delegate: SwipeDelegate
		} // TPListView: messagesList

		ColumnLayout {
			spacing: 10
			Layout.preferredHeight: childrenRect.height
			Layout.fillWidth: true

			TPLabel {
				text: qsTr("Chat with ...")
				font: AppGlobals.smallFont
				Layout.fillWidth: true
			}

			TPTextInput {
				id: txtSearch
				showClearTextButton: true
				Layout.fillWidth: true
				onTextChanged: chatList.applyFilter(text);
			}

			TPCoachesAndClientsList {
				id: chatList
				listClients: true
				listCoaches: true
				Layout.preferredHeight: onlineMsgsDlg.maxHeight - 2 * (AppSettings.itemDefaultHeight + 10)
				Layout.preferredWidth: parent.width
				Layout.fillWidth: true

				property string selectedChat

				onItemSelected: (userRow) => {
					selectedChat = AppUserModel.userName(userRow);
					txtSearch.text = selectedChat;
					onlineMsgsDlg.openChat(selectedChat);
				}
			} //TPCoachesAndClientsList
		}
	} //StackLayout

	TPButton {
		imageSource: "chat.png"
		width: AppSettings.itemDefaultHeight
		height: width
		visible: mainLayout.visible && mainLayout.currentIndex !== 2

		anchors {
			bottom: mainLayout.bottom
			bottomMargin: 10
			right: mainLayout.right
			rightMargin: 10
		}

		onClicked: mainLayout.currentIndex = 2;
	}

	function openChat(user_name: string): void {
		AppMessages.openChat(user_name);
		mainLayout.currentIndex = AppMessages.count > 0 ? 1 : 0;
	}
}
