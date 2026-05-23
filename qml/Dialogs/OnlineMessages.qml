pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.User

TPPopup {
	objectName: "onlineMessages"
	id: onlineMsgsDlg
	keepAbove: true
	backGroundImage: fullDialogVisible ? ":/images/backgrounds/backimage-messages.jpg" : ""
	configFieldName: "onlineMessagesDialogPosition"
	defaultCoordinates: Qt.point(AppSettings.pageWidth - 80, realPageY() + 180)
	mouseItem: fullDialogVisible ? topBar : mainIcon
	useAlternateBackground: !fullDialogVisible
	defaultBackgroundColor: "transparent"
	globalPopup: AppSettings.showOnlineMessagesDialog
	width: savedSize.width
	height: savedSize.height

//private:
	property bool fullDialogVisible: savedSize.width > mainIcon.width
	property int mainIconUserDefinedX: x
	property int mainIconUserDefinedY: y
	readonly property int dlgMaxWidth: AppSettings.pageWidth * 0.8
	readonly property int maxHeight: AppSettings.pageHeight * 0.5
	readonly property size savedSize: AppSettings.getCustomValue("onlineMessagesDialogSize", Qt.size(mainIcon.width, mainIcon.height))

	onMouseItemClicked: (mouse) => {
		if (fullDialogVisible) {
			if (ItemManager.AppPagesManager.isPopupAboveAllOthers(onlineMsgsDlg)) {
				onlineMsgsDlg.mainIconUserDefinedY = onlineMsgsDlg.y;
				shrink.start();
			}
			else
				ItemManager.AppPagesManager.raisePopup(onlineMsgsDlg);
		}
		else {
			onlineMsgsDlg.mainIconUserDefinedX = onlineMsgsDlg.x;
			onlineMsgsDlg.mainIconUserDefinedY = onlineMsgsDlg.y;
			expand.start();
		}
	}

	ParallelAnimation {
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
			AppSettings.setCustomValue("onlineMessagesDialogSize", Qt.size(onlineMsgsDlg.width, onlineMsgsDlg.height));
			AppSettings.setCustomValue(onlineMsgsDlg.configFieldName, Qt.point(onlineMsgsDlg.x, onlineMsgsDlg.y));
		}
	}

	ParallelAnimation {
		id: expand
		alwaysRunToEnd: true

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "width"
			to: onlineMsgsDlg.dlgMaxWidth
			duration: 200
			easing.type: Easing.InQuad
		}

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "height"
			to: topBar.height + mainLayout.height
			duration: 200
			easing.type: Easing.InQuad
		}

		onFinished: {
			if ((onlineMsgsDlg.x + onlineMsgsDlg.width) > AppSettings.pageWidth)
				onlineMsgsDlg.x = AppSettings.pageWidth - onlineMsgsDlg.width - 10;
			onlineMsgsDlg.fullDialogVisible = true;
			AppSettings.setCustomValue("onlineMessagesDialogSize", Qt.size(onlineMsgsDlg.width, onlineMsgsDlg.height));
			AppSettings.setCustomValue(onlineMsgsDlg.configFieldName, Qt.point(onlineMsgsDlg.x, onlineMsgsDlg.y));
		}
	}

	TPBackRec {
		id: transparentBackground
		backColor: "transparent"
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

			delegate: SwipeDelegate {
				id: delegateItem
				swipe.enabled: msgSticky
				clip: true
				padding: 0
				spacing: 5
				width: messagesList.width

				required property int index
				required property int msgId
				required property string msgTitle
				required property string msgText
				required property string msgIcon
				required property string msgFileName
				required property string msgExtraInfoText
				required property string msgExtraInfoIcon
				required property string msgDate
				required property string msgTime
				required property list<string> msgActions
				required property bool msgSticky
				required property bool msgHasActions

				property bool expanded: false

				background: Rectangle {
					opacity: 0.8
					color: delegateItem.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2
				}

				contentItem: ColumnLayout {
					id: messageLayout
					spacing: 5
					opacity: 1 + delegateItem.swipe.position

					Item {
						Layout.preferredWidth: delegateItem.width - 10
						Layout.preferredHeight: AppSettings.itemExtraLargeHeight
						Layout.leftMargin: 5
						Layout.rightMargin: 5

						TPImage {
							id: msgImage
							source: delegateItem.msgIcon
							imageSizeFollowControlSize: true
							keepAspectRatio: true
							fullWindowView: false
							dropShadow: false
							visible: delegateItem.msgIcon.length > 0
							width: AppSettings.itemExtraLargeHeight
							height: AppSettings.itemExtraLargeHeight
							anchors {
								top: parent.top
								left: parent.left
							}
						}


						TPLabel {
							id: lblTitle
							text: delegateItem.msgTitle + "<br>" + delegateItem.msgDate + "  " + delegateItem.msgTime
							font: AppGlobals.smallFont
							singleLine: false
							verticalAlignment: Label.AlignTop
							height: AppSettings.itemExtraLargeHeight
							width: parent.width - msgImage.width - extraInfoImg.width - btnFoldIcon.width

							anchors {
								left: msgImage.right
								top: parent.top
							}
						}

						TPImage {
							id: extraInfoImg
							source: delegateItem.msgExtraInfoIcon
							visible: delegateItem.msgExtraInfoText.length > 0
							width: visible ? AppSettings.itemSmallHeight : 0
							height: visible ? AppSettings.itemSmallHeight : 0

							anchors {
								verticalCenter: parent.verticalCenter
								left: lblTitle.right
							}

							TPLabel {
								text: delegateItem.msgExtraInfoText
								minimumPixelSize: AppSettings.smallFontSize * 0.7
								z: 1
								width: parent.width * 0.5
								height: parent.height * 0.8
								anchors.centerIn: parent
							}
						}

						TPImage {
							id: btnFoldIcon
							source: delegateItem.expanded ? "fold-up.png" : "fold-down.png"
							dropShadow: false
							width: AppSettings.itemSmallHeight
							height: AppSettings.itemSmallHeight

							anchors {
								verticalCenter: parent.verticalCenter
								left: lblTitle.right
							}
						}
					}

					Loader {
						id: msgTextLoader
						asynchronous: true
						active: delegateItem.msgText !== ""
						visible: delegateItem.expanded
						Layout.fillWidth: true
						Layout.preferredHeight: active ? _label.contentHeight : 0
						Layout.leftMargin: 10
						Layout.rightMargin: 10

						property TPLabel _label
						sourceComponent: TPLabel {
							id: lblMessage
							text: delegateItem.msgText
							font: AppGlobals.smallFont
							singleLine: false
							Component.onCompleted: msgTextLoader._label = this;
						}
					}

					Loader {
						id: fileViewerLoader
						asynchronous: true
						active: delegateItem.msgFileName.length > 0
						visible: delegateItem.expanded
						Layout.preferredWidth: active ? Math.max(100, _file_viewer.minimumWidth) : 0
						Layout.preferredHeight: active ? Math.max(100, _file_viewer.minimumHeight) : 0

						property TPFileViewer _file_viewer
						sourceComponent: TPFileViewer {
							mediaSource: delegateItem.msgFileName
							onRemovalRequested: AppMessages.removeMessage(delegateItem.msgId);
							Component.onCompleted: fileViewerLoader._file_viewer = this;
						}
					}

					Loader {
						id: actionsLoader
						asynchronous: true
						active: delegateItem.msgActions.length > 0
						visible: delegateItem.expanded
						Layout.fillWidth: true
						Layout.preferredHeight: active ? _layout.childrenRect.height : 0

						property GridLayout _layout

						sourceComponent: GridLayout {
							id: _layout
							columns: 2
							columnSpacing: 2
							rowSpacing: 5

							readonly property int maxButtonWidth: (onlineMsgsDlg.dlgMaxWidth - 25)/3
							readonly property int msgIndex: delegateItem.index

							Repeater {
								id: actionsRepeater
								model: delegateItem.msgActions

								delegate: TPButton {
									text: delegateItem.msgActions[index]
									width: constrainSize ? _layout.maxButtonWidth : preferredWidth
									autoSize: !constrainSize
									rounded: false
									Layout.alignment: Qt.AlignCenter
									onClicked: AppMessages.execAction(_layout.msgIndex, index);

									required property int index
									property bool constrainSize: true
									property int row
									property int col
								}

								//items are added in reverse order(from last to first) from TPMessages::insertAction call order
								onItemAdded: (index, item) => {
									if (index === 0) {
										const n_items = actionsRepeater.model.count;
										let row_width = 0, row = 0, col = 0;
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
									}
								} //onItemAdded
							} //Repeater

							Component.onCompleted: actionsLoader._layout = this;
						} //sourceComponent: GridLayout
					} //Loader
				} //contentItem: Column

				onClicked: delegateItem.expanded = !delegateItem.expanded;

				swipe.right: Rectangle {
					id: removeBackground
					enabled: !delegateItem.msgSticky
					color: SwipeDelegate.pressed ? Qt.darker("tomato", 1.1) : "tomato"
					opacity: delegateItem.swipe.complete ? 0.8 : 0-delegateItem.swipe.position
					anchors.fill: parent

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

				onItemSelected: (userIdx) => {
					txtSearch.text = AppUserModel.userName(userIdx);
					onlineMsgsDlg.openChat(userIdx);
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
