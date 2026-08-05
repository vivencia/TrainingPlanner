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
	backGroundImage: fullDialogVisible ? ":/images/backgrounds/backimage-messages.jpg" : ""
	configFieldName: "onlineMessagesDialogPosition"
	defaultCoordinates: Qt.point(AppSettings.pageWidth - 80, 180)
	mouseItem: fullDialogVisible ? topBar : mainIcon
	useAlternateBackground: !fullDialogVisible
	defaultBackgroundColor: "transparent"
	showBehavior: AppSettings.showOnlineMessagesDialog ? TPPopup.ALWAYS_VISIBLE : TPPopup.PARENT_PAGE_ACTIVE
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
			if (ItemManager.appPagesManager.isPopupAboveAllOthers(onlineMsgsDlg)) {
				onlineMsgsDlg.mainIconUserDefinedY = onlineMsgsDlg.y;
				shrink.start();
			}
			else
				ItemManager.appPagesManager.raisePopup(onlineMsgsDlg);
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
		currentIndex: AppMessages.messagesModel.hasMessage ? 1 : 0

		anchors {
			top: topBar.bottom
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		TPLabel {
			text: qsTr("No messages")
			useBackground: true
			horizontalAlignment: Qt.AlignHCenter
			font: AppGlobals.largeFont
			Layout.preferredHeight: onlineMsgsDlg.maxHeight / 2
			Layout.fillWidth: true
		}

		TreeView {
			id: messagesList
			model: AppMessages.messagesModel
			clip: true
			Layout.fillWidth: true
			Layout.fillHeight: true

			selectionModel: ItemSelectionModel {}

			delegate: TreeViewDelegate {
				id: delegateItem
				implicitWidth: onlineMsgsDlg.width
				implicitHeight: messageLayout.childrenRect.height * 1.1
				indentation: 10

				required property TPMessage tpMessage
				property bool collapsed: false

				TapHandler {
					target: delegateItem
					parent: delegateItem

					onSingleTapped: {
						const index = messagesList.index(delegateItem.model.row, delegateItem.model.column);
						messagesList.selectionModel.setCurrentIndex(index, ItemSelectionModel.NoUpdate);
						messagesList.toggleExpanded(delegateItem.model.row);
					}
				}

				background: Rectangle { // Background rectangle enabled to show the alternative row colors
					id: background
					opacity: 0.8
					anchors.fill: parent
					color: {
						let _color = delegateItem.model.row % 2 !== 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2;
						if (delegateItem.model.row === messagesList.currentRow)
							_color = Qt.darker(_color, 1.2);
						return _color;
					}
				}

				indicator: Item {}
				TPLabel {
					id: indicator
					anchors {
						left: parent.left
						leftMargin: delegateItem.padding + ((delegateItem.depth + 1) * delegateItem.indentation)
						verticalCenter: parent.verticalCenter
					}
					text: delegateItem.expanded ? "▼" : "▶"
					visible: delegateItem.isTreeNode && delegateItem.hasChildren
				}

				contentItem: Item {}
				ColumnLayout {
					id: messageLayout
					spacing: 5
					x: indicator.x + indicator.width + delegateItem.padding
					width: delegateItem.width - delegateItem.padding - x

					Item {
						Layout.fillWidth: true
						Layout.preferredHeight: AppSettings.itemExtraLargeHeight
						Layout.leftMargin: 5
						Layout.rightMargin: 5

						TPImage {
							id: msgImage
							source: delegateItem.tpMessage.icon
							imageSizeFollowControlSize: true
							keepAspectRatio: true
							fullWindowView: false
							dropShadow: false
							visible: delegateItem.tpMessage.hasIcon
							width: AppSettings.itemExtraLargeHeight
							height: AppSettings.itemExtraLargeHeight
							anchors {
								verticalCenter: parent.verticalCenter
								left: parent.left
							}
						}

						TPLabel {
							id: lblTitle
							text: delegateItem.tpMessage.title + "<br>" + delegateItem.tpMessage.dateTime
							font: AppGlobals.smallFont
							singleLine: false
							verticalAlignment: Label.AlignTop
							height: AppSettings.itemExtraLargeHeight
							width: parent.width - msgImage.width - extraInfoImg.width - btnFoldIcon.width

							anchors {
								top: parent.top
								left: msgImage.right
								right: delegateItem.tpMessage.hasExtraImage
																	? extraInfoImg.left : btnFoldIcon.left
								margins: 3
							}
						}

						TPImage {
							id: extraInfoImg
							source: delegateItem.tpMessage.extraImage
							visible: delegateItem.tpMessage.hasExtraImage
							width: visible ? AppSettings.itemSmallHeight : 0
							height: visible ? AppSettings.itemSmallHeight : 0

							anchors {
								verticalCenter: parent.verticalCenter
								left: lblTitle.right
							}

							TPLabel {
								text: delegateItem.tpMessage.extraInfo
								minimumPixelSize: AppSettings.smallFontSize * 0.7
								z: 1
								width: parent.width * 0.5
								height: parent.height * 0.8
								anchors.centerIn: parent
							}
						}

						TPImage {
							id: btnFoldIcon
							source: delegateItem.collapsed ? "fold-up.png" : "fold-down.png"
							visible: delegateItem.tpMessage.text.length > 0
							width: AppSettings.itemSmallHeight
							height: AppSettings.itemSmallHeight

							anchors {
								top: parent.top
								right: parent.right
							}
						}

						MouseArea {
							anchors.fill: parent
							onClicked: delegateItem.collapsed = !delegateItem.collapsed;
						}
					}

					TPLabel {
						id: lblMessage
						text: delegateItem.tpMessage.text
						font: AppGlobals.smallFont
						visible: delegateItem.collapsed
						singleLine: false
						Layout.fillWidth: true
						Layout.leftMargin: 10
						Layout.rightMargin: 10
					}

					Loader {
						id: fileViewerLoader
						asynchronous: true
						active: delegateItem.tpMessage.fileOps !== null
						visible: delegateItem.collapsed && _file_viewer
						Layout.alignment: Qt.AlignHCenter
						Layout.preferredWidth: _file_viewer ? _file_viewer.minimumWidth : parent.width
						Layout.preferredHeight: _file_viewer ? _file_viewer.minimumHeight : AppSettings.itemDefaultHeight

						property TPFileViewer _file_viewer: null
						sourceComponent: TPFileViewer {
							fileOps: delegateItem.tpMessage.fileOps
							Component.onCompleted: fileViewerLoader._file_viewer = this;
						}
					}

					Loader {
						id: actionsLoader
						asynchronous: true
						active: delegateItem.tpMessage.actionCount > 0
						visible: delegateItem.collapsed
						Layout.fillWidth: true
						Layout.leftMargin: 5
						Layout.rightMargin: 5
						Layout.preferredHeight: active ? _layout.childrenRect.height : 0

						property GridLayout _layout

						onActiveChanged: {
							if (active) {
								for (let i = 0; i < delegateItem.tpMessage.actionCount; ++i) {
									let component, item;
									switch (delegateItem.tpMessage.actionType()) {
									case TPMessage.AT_BUTTON:
										component = Qt.createComponent("TpQml.Widgets", TPButton, { text:
																delegateItem.tpMessage.actionLabel(index) });
										item = component.createObject(actionsLayout, {});
										break;
									case TPMessage.AT_CHECKBOX:
										component = Qt.createComponent("TpQml.Widgets", TPRadioButtonOrCheckBox, { text:
											delegateItem.tpMessage.actionLabel(index), boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX});
										item = component.createObject(actionsLayout, {});
										break;
									case TPMessage.AT_RADIO:
										component = Qt.createComponent("TpQml.Widgets", TPRadioButtonOrCheckBox, { text:
											delegateItem.tpMessage.actionLabel(index), boxType: TPRadioButtonOrCheckBox.TP_RADIOBOX});
										item = component.createObject(actionsLayout, {});
										break;
									case TPMessage.AT_NONE:
										continue;
									}
									if (item)
										actionsLayout.addItem(item, index);
								}
							}
						}

						sourceComponent: GridLayout {
							id: actionsLayout
							columns: 2
							columnSpacing: 2
							rowSpacing: 5

							property int _row: 0
							property int _col: 0
							property list<int> _row_width: [0]
							readonly property int _max_row_width: width - 10

							function addItem(item: Item, index: int): void {
								item.Layout.column = _col;
								item.Layout.row = _row;
								if (item.width >= _max_row_width * 0.8) { //too big to shrink
									_row_width.push(0);
									++_row
									_col = 0;
								} else {
									if (item.width > _max_row_width * 0.5) //big, but shrinkable
										item.width = _max_row_width * 0.5
									_row_width[_row] += item.width;
									++_col;
									if (_row_width[_row] >= _max_row_width * 0.9) {
										_row_width.push(0);
										++_row
										_col = 0;
									}
								}
							}

							Component.onCompleted: actionsLoader._layout = this;
						} //sourceComponent: GridLayout
					} //Loader: actionsLoader
				} //contentItem: ColumnLayout
			} //delegate: TreeViewDelegate
		} // TPListView: messagesList

		Item {
			id: newChatOrMessagePane
			Layout.fillHeight: true
			Layout.fillWidth: true

			property int _useridx: -1

			TPCoachesAndClientsList {
				id: chatList
				listClients: true
				listCoaches: true
				listConfirmed: true

				anchors {
					top: parent.top
					left: parent.left
					right: parent.right
					bottom: buttonsLayout.top
					margins: 5
				}

				onItemSelected: (userIdx) => {
					txtSearch.text = AppUserModel.userName(userIdx);
					newChatOrMessagePane._useridx = userIdx;
				}
			} //TPCoachesAndClientsList

			RowLayout {
				id: buttonsLayout
				spacing: 10

				anchors {
					left: parent.left
					right: parent.right
					bottom: parent.bottom
					margins: 5
				}

				TPButton {
					text: qsTr("Chat")
					sourceImage: "chat.png"
					enabled: newChatOrMessagePane._useridx > 0
					Layout.preferredWidth: preferredWidth
					Layout.maximumWidth: parent.width / 2
					onClicked: onlineMsgsDlg.openChat(newChatOrMessagePane._useridx);
				}
				TPButton {
					text: qsTr("Send message")
					sourceImage: "send-message.png"
					enabled: newChatOrMessagePane._useridx > 0
					Layout.preferredWidth: preferredWidth
					Layout.maximumWidth: parent.width / 2
					onClicked: onlineMsgsDlg.newMessage(newChatOrMessagePane._useridx);
				}
			}
		}
	} //StackLayout

	Rectangle {
		color: AppSettings.primaryColor
		opacity: 0.6
		width: AppSettings.itemLargeHeight
		height: width
		radius: width / 2

		anchors {
			bottom: mainLayout.bottom
			bottomMargin: mainLayout.currentIndex !== 2 ? 10 : AppSettings.itemDefaultHeight + 15
			right: mainLayout.right
			rightMargin: 10
		}

		TPButton {
			imageSource: mainLayout.currentIndex !== 2 ? "add-new.png" : "revert.png"
			width: AppSettings.itemDefaultHeight
			height: width
			visible: onlineMsgsDlg.fullDialogVisible
			anchors.centerIn: parent
			onClicked: mainLayout.currentIndex = mainLayout.currentIndex !== 2
														? 2 : (AppMessages.messagesModel.hasMessage ? 1 : 0);
		}
	}

	function openChat(user_idx: int): void {
		AppMessages.openChat(user_idx);
		mainLayout.currentIndex = 1;
	}

	function newMessage(user_idx: int): void {
		AppMessage.openNewMessageDialog(user_idx);
		mainLayout.currentIndex = 1;
	}
}
