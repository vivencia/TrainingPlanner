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
	x: appSettings.pageWidth - 80
	finalYPos: 180
	width: mainIcon.width
	height: mainIcon.height
	backgroundRec.visible: fullDialogVisible
	backGroundImage: ":/images/backgrounds/backimage-messages.jpg"

	property bool fullDialogVisible: false
	property int mainIconUserDefinedX: x
	property int mainIconUserDefinedY: y
	readonly property int dlgMaxWidth: appSettings.pageWidth * 0.8
	readonly property int maxHeight: appSettings.pageHeight * 0.5

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
			onMouseClicked: (hold_clicked) => {
				if (!hold_clicked) {
					mainIconUserDefinedX = onlineMsgsDlg.x;
					mainIconUserDefinedY = onlineMsgsDlg.y;
					expand.start();
				}
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
				if (mainwindow.appPagesModel.isPopupAboveAllOthers(onlineMsgsDlg)) {
					mainIconUserDefinedY = onlineMsgsDlg.y;
					shrink.start();
				}
				else
					mainwindow.appPagesModel.raisePopup(onlineMsgsDlg);
			}
		}
	}

	StackLayout {
		id: mainLayout
		visible: fullDialogVisible
		currentIndex: appMessages ? appMessages.count > 0 ? 1 : 0 : 0
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

		TPListView {
			id: messagesList
			model: appMessages
			Layout.fillWidth: true
			Layout.fillHeight: true

			property int messagesHeight: 0

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
								imageSizeFollowControlSize: true
								keepAspectRatio: true
								fullWindowView: false
								dropShadow: false
								width: appSettings.itemSmallHeight
								height: width
							}

							TPLabel {
								text: msgdate + "  " + msgtime
								font: AppGlobals.smallFont
								height: appSettings.itemSmallHeight
								Layout.leftMargin: appSettings.itemSmallHeight
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
								autoSize: !constrainSize
								rounded: false
								Layout.alignment: Qt.AlignCenter
								onClicked: appMessages.execAction(actionsLayout.msgIndex, index);

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
										if (col === 0 || (row_width < dlgMaxWidth - 5)) {
											itemAt(i).Layout.column = col;
											itemAt(1).Layout.row = row;
											col++
										}
										else {
											if (itemAt(i).width >= itemAt(i-1).width)
												itemAt(i).constrainSize = true;
											else
												itemAt(i-1).constrainSize = true;
											if (itemAt(i).width + itemAt(i-1).width >= dlgMaxWidth - 5) {
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
									messagesList.messagesHeight += actionsLayout.childrenRect.height + lblMessage.height + messageTextLayout.height;
								}
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
