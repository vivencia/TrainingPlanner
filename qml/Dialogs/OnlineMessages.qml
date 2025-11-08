import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: onlineMsgsDlg
	keepAbove: true
	showTitleBar: false
	closeButtonVisible: false
	disableMouseHandling: true
	enableEffects: false
	x: appSettings.pageWidth - 80
	finalYPos: 180
	width: mainIcon.width
	height: mainIcon.height
	backgroundRec: backRec

	property bool fullDialogVisible: false
	property int mainIconUserDefinedX: x
	property int mainIconUserDefinedY: y
	readonly property int dlgMaxWidth: appSettings.pageWidth * 0.8
	readonly property int maxHeight: appSettings.pageHeight * 0.5

	TPBackRec {
		id: backRec
		useImage: fullDialogVisible
		visible: fullDialogVisible
		sourceImage: ":/images/backgrounds/backimage-messages.jpg"
		image_size: Qt.size(dlgMaxWidth, maxHeight)
		radius: 8
		layer.enabled: true
		anchors.fill: parent
	}

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
			onlineMsgsDlg.x = mainIconUserDefinedX;
			onlineMsgsDlg.y = mainIconUserDefinedY;
			fullDialogVisible = false;
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
			if ((onlineMsgsDlg.x + onlineMsgsDlg.width) > appSettings.pageWidth)
				onlineMsgsDlg.x = appSettings.pageWidth - onlineMsgsDlg.width - 10;
			/*if ((onlineMsgsDlg.y + onlineMsgsDlg.height) > appSettings.pageHeight) {
				console.log(onlineMsgsDlg.y, onlineMsgsDlg.height, appSettings.pageHeight);
				onlineMsgsDlg.y = appSettings.pageHeight - onlineMsgsDlg.height - 10;
			}*/
			fullDialogVisible = true;
		}
	}

	TPImage {
		id: mainIcon
		source: "messages"
		width: appSettings.itemExtraLargeHeight
		height: width
		visible: !fullDialogVisible
		anchors {
			verticalCenter: parent.verticalCenter
			horizontalCenter: parent.horizontalCenter
		}

		TPMouseArea {
			movingWidget: mainIcon
			movableWidget: onlineMsgsDlg
			enabled: !fullDialogVisible

			onPositionChanged: (mouse) => positionChangedFunction(mouse);
			onMouseClicked: {
				mainIconUserDefinedX = onlineMsgsDlg.x;
				mainIconUserDefinedY = onlineMsgsDlg.y;
				expand.start();
			}
		}
	}

	TPLabel {
		id: topBar
		text: qsTr("Messages")
		visible: fullDialogVisible
		height: appSettings.itemLargeHeight
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
			width: appSettings.itemDefaultHeight
			height: width

			anchors {
				left: topBar.right
				verticalCenter: topBar.verticalCenter;
			}
		}

		TPMouseArea {
			movingWidget: parent
			movableWidget: onlineMsgsDlg

			onPositionChanged: (mouse) => positionChangedFunction(mouse);
			onMouseClicked: {
				mainIconUserDefinedY = onlineMsgsDlg.y;
				shrink.start();
			}
		}
	}

	StackLayout {
		id: mainLayout
		visible: fullDialogVisible
		currentIndex: appMessages.count > 0 ? 1 : 0
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
			height: maxHeight / 2
			Layout.fillWidth: true
		}

		ListView {
			id: messagesList
			contentHeight: availableHeight
			contentWidth: availableWidth
			spacing: 5
			reuseItems: true
			clip: true
			model: appMessages
			Layout.fillWidth: true
			Layout.fillHeight: true

			property int messagesHeight: 0

			ScrollBar.vertical: ScrollBar {
				policy: ScrollBar.AsNeeded
				active: true; visible: messagesList.contentHeight > messagesList.height
			}

			delegate: SwipeDelegate {
				id: delegateItem
				swipe.enabled: sticky
				clip: true
				padding: 0
				spacing: 5
				width: messagesList.width

				property bool showActions: false

				function setShowActions(show: bool) {
					if (show !== showActions) {
						messagesList.messagesHeight -= actionsLayout.childrenRect.height + lblMessage.height;
						showActions = show;
						messagesList.messagesHeight += actionsLayout.childrenRect.height + lblMessage.height;
					}
				}

				background: Rectangle {
					opacity: 0.8
					color: index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
				}

				contentItem: Column {
					id: messageLayout
					spacing: 5
					padding: 5
					opacity: 1 + swipe.position

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
								source: msgicon
								dropShadow: false
								width: 20
								height: 20
							}

							TPLabel {
								text: msgdate + "  " + msgtime
								font: AppGlobals.smallFont
								height: 15
								Layout.leftMargin: 20
							}

							Item {
								width: appSettings.itemSmallHeight
								height: width
								visible: extraInfo.length > 0

								TPImage {
									id: extraInfoImg
									source: extraInfoIcon
									anchors.fill: parent
								}
								TPLabel {
									text: extraInfo
									horizontalAlignment: Text.AlignHCenter
									minimumPixelSize: appSettings.smallFontSize * 0.7
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
								width: 18
								height: 18
								Layout.leftMargin: 30
							}
						}
					}

					TPLabel {
						id: lblMessage
						text: labelText
						elide: delegateItem.showActions ? Text.ElideNone : Text.ElideRight
						wrapMode: delegateItem.showActions ? Text.WordWrap : Text.NoWrap
						singleLine: !delegateItem.showActions
						width: onlineMsgsDlg.dlgMaxWidth - 10
						height: delegateItem.showActions ? contentHeight : appSettings.itemDefaultHeight
						Layout.leftMargin: 20

						MouseArea {
							anchors.fill: parent
							onClicked: {
								if (hasActions)
									delegateItem.setShowActions(!delegateItem.showActions);
								else
									appMessages.itemClicked(index);
							}
						}
					}

					GridLayout {
						id: actionsLayout
						columns: 2
						visible: delegateItem.showActions
						columnSpacing: 2
						rowSpacing: 5
						Layout.maximumWidth: dlgMaxWidth
						Layout.minimumWidth: dlgMaxWidth

						readonly property int maxButtonWidth: (dlgMaxWidth - 25)/3
						readonly property int msgIndex: index

						Repeater {
							id: actionsRepeater
							model: actions

							delegate: TPButton {
								text: actions[index]
								width: constrainSize ? actionsLayout.maxButtonWidth : preferredWidth
								autoSize: constrainSize
								rounded: false
								Layout.leftMargin: 0
								Layout.rightMargin: 0
								onClicked: appMessages.execAction(actionsLayout.msgIndex, index);

								required property int index
								property bool constrainSize: false
							}

							//items are added in reverse order(last to first)
							onItemAdded: (index, item) => {
								if (index % 2 === 0) {
									let itemsWidths = new Array(2);
									itemsWidths[0] = item;
									itemsWidths[1] = null;
									let rowWidth = item.width;

									if (actionsRepeater.itemAt(index+1)) {
										const item1 = actionsRepeater.itemAt(index+1);
										if (item1.width > itemsWidths[0].width) {
											itemsWidths[1] = itemsWidths[0];
											itemsWidths[0] = item1;
										}
										else
											itemsWidths[1] = item1;
										rowWidth += item1.width;
									}
									/*if (actionsRepeater.itemAt(index+2)) {
										const item2 = actionsRepeater.itemAt(index+2);
										if (item2.width > itemsWidths[1].width) {
											itemsWidths[2] = itemsWidths[1];
											itemsWidths[1] = itemsWidths[0];
											itemsWidths[0] = item2;
										}
										else if (item2.width > itemsWidths[0].width) {
											itemsWidths[2] = itemsWidths[1];
											itemsWidths[1] = item2;
										}
										else
											itemsWidths[2] = item2;
										rowWidth += item2.width;
									}*/

									if (rowWidth >= dlgMaxWidth) {
										rowWidth -= itemsWidths[0].width;
										itemsWidths[0].constrainSize = true;
										rowWidth += itemsWidths[0].width;
										if (rowWidth >= dlgMaxWidth) {
											rowWidth -= itemsWidths[1].width;
											itemsWidths[1].constrainSize = true;
											rowWidth += itemsWidths[1].width;
										}
									}
									item.Layout.leftMargin = (dlgMaxWidth - rowWidth)/2;
								}
								if (index === 0)
									messagesList.messagesHeight += actionsLayout.childrenRect.height + lblMessage.height + messageTextLayout.height;
							}
						}
					}
				} //ColumnLayout

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
				swipe.onCompleted: appMessages.removeMessage(index);
			} //delegate
		} // messagesList

		ColumnLayout {
			spacing: 10
			height: childrenRect.height
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
				height: maxHeight - 2 * (appSettings.itemDefaultHeight + 10)
				width: parent.width
				Layout.fillWidth: true

				property string selectedChat

				onItemSelected: (userRow) => {
					selectedChat = userModel.userName(userRow);
					txtSearch.text = selectedChat;
					openChat(selectedChat);
				}
			} //TPCoachesAndClientsList
		}
	} //StackLayout

	TPButton {
		imageSource: "chat.png"
		width: appSettings.itemDefaultHeight
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
		appMessages.openChat(user_name);
		mainLayout.currentIndex = appMessages.count > 0 ? 1 : 0;
	}
}
