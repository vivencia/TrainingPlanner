pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.User

TPPopup {
	id: onlineMsgsDlg
	objectName: "onlineMsgsDlg"
	keepAbove: true
	backGroundImage: _minimized ? "" : ":/images/backgrounds/backimage-messages.jpg"
	useAlternateBackground: !_minimized
	defaultBackgroundColor: "transparent"
	mouseItem: _minimized ? mainIcon : titleBar
	showBehavior: AppSettings.showOnlineMessagesDialog ? TPPopup.ALWAYS_VISIBLE : TPPopup.PARENT_PAGE_ACTIVE
	defaultCoordinates: Qt.point(80, 180)
	normal_size: Qt.size(AppSettings.pageWidth * 0.8, AppSettings.pageWidth * 0.8)
	minimized_size: Qt.size(mainIcon.width, mainIcon.height)
	resizeable: true
	savePopupState: true
	show_minimize_button: false

	onMouseItemClicked: (mouse) => {
		if (_minimized)
			titleBar.restore();
		else
			titleBar.minimize();
	}

	on_MinimizedChanged: {
		if (titleBar != null)
			titleBar.visible = !_minimized;
	}
	onOpened: titleBar.visible = !_minimized;

	TPBackRec {
		id: transparentBackground
		backColor: "transparent"
	}

	TPImage {
		id: mainIcon
		source: "messages"
		width: AppSettings.itemExtraLargeHeight
		height: width
		visible: onlineMsgsDlg._minimized

		anchors {
			verticalCenter: parent.verticalCenter
			horizontalCenter: parent.horizontalCenter
		}
	}

	TPImage {
		id: smallIcon
		source: "messages"
		dropShadow: false
		visible: !onlineMsgsDlg._minimized
		width: AppSettings.itemDefaultHeight
		height: width

		anchors {
			top: parent.top
			left: parent.left
			leftMargin: 10
		}
	}

	TPLabel {
		id: topBar
		text: qsTr("Messages")
		visible: !onlineMsgsDlg._minimized
		horizontalAlignment: Text.AlignHCenter

		anchors {
			left: smallIcon.right
			leftMargin: 10
			verticalCenter: smallIcon.verticalCenter
		}
	}

	StackLayout {
		id: mainLayout
		visible: !onlineMsgsDlg._minimized
		currentIndex: AppMessages.messagesModel.hasMessage ? 1 : 0

		anchors {
			margins: 0
			topMargin: onlineMsgsDlg.titleBarHeight
			top: parent.top
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
			contentHeight: onlineMsgsDlg.availableHeight * 1.1
			contentWidth: onlineMsgsDlg.availableWidth
			reuseItems: false
			clip: true
			Layout.fillWidth: true
			Layout.fillHeight: true

			selectionModel: ItemSelectionModel {}

			ScrollBar.vertical: ScrollBar {
				policy: ScrollBar.AsNeeded
				active: true
			}

			delegate: Item {
				id: delegateItem
				implicitWidth: onlineMsgsDlg.width
				implicitHeight: headerWidget.height

				required property TPMessage tpMessage
				required property bool expanded
				required property int index
				readonly property bool hasChildren: tpMessage.childCount > 0
				readonly property int indentation: -10 + tpMessage.depth * 12
				readonly property int widthAvailable: width - indentation - 5
				property bool collapsed: false
				property int delegateHeight: 0

				onCollapsedChanged: {
					implicitHeight = (collapsed ? delegateHeight + headerWidget.height : headerWidget.height);
					messagesList.forceLayout(); //force a repositioning of all the visible items
				}

				TPBackRec {
					radius: 8
					opacity: 0.8
					enableShadow: true
					backColor: {
						if (tpMessage.type === TPMessage.MT_PHANTON) return "transparent";
						let _color = tpMessage.row % 2 !== 0 ? AppSettings.primaryDarkColor : AppSettings.primaryColor;
						if (tpMessage.depth > 0)
							_color = Qt.lighter(_color, tpMessage.depth * 1.2 + (tpMessage.depth * 0.1));
						return _color;
					}
					anchors {
						fill: parent
						margins: 2
						leftMargin: delegateItem.indentation
					}

					Frame {
						id: headerWidget
						width: messagesList.width
						height: AppSettings.itemExtraLargeHeight + 5
						anchors {
							top: parent.top
							left: parent.left
						}

						background: Rectangle {
							color: "transparent"
							border.color: "transparent"
						}

						TPLabel {
							id: indicator
							text: delegateItem.expanded ? "▼" : "▶"
							width: AppSettings.itemSmallHeight
							visible: delegateItem.hasChildren
							anchors {
								margins: 0
								left: parent.left
								verticalCenter: parent.verticalCenter
							}
						}

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
								left: indicator.visible ? indicator.right : parent.left
							}
						}

						TPLabel {
							id: lblTitle
							text: delegateItem.tpMessage.title + "<br>" + delegateItem.tpMessage.dateTime
							font: AppGlobals.smallFont
							singleLine: false
							verticalAlignment: Label.AlignTop
							height: AppSettings.itemExtraLargeHeight

							anchors {
								verticalCenter: parent.verticalCenter
								left: msgImage.right
								right: delegateItem.tpMessage.hasExtraImage ? extraInfoImg.left : btnFoldIcon.left
								margins: 0
								leftMargin: 5
							}
						}

						TPImage {
							id: extraInfoImg
							source: delegateItem.tpMessage.extraImage
							visible: delegateItem.tpMessage.hasExtraImage
							width: AppSettings.itemSmallHeight
							height: width

							anchors {
								verticalCenter: parent.verticalCenter
								right: btnFoldIcon.left
								margins: 0
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
							visible: delegateItem.tpMessage.text.length > 0 || delegateItem.tpMessage.actionCount > 0
							width: AppSettings.itemSmallHeight
							height: AppSettings.itemSmallHeight

							anchors {
								top: parent.top
								right: parent.right
								margins: Qt.platform.os !== "android" ? 10 : 4
							}
						}

						MouseArea {
							anchors.fill: parent
							enabled: btnFoldIcon.enabled
							onClicked: (mouse) => {
								let _mouse_pos_within_widget = parent.mapToItem(parent, mouse.x, mouse.y);
								if (_mouse_pos_within_widget.x >= msgImage.x)
									delegateItem.collapsed = !delegateItem.collapsed;
								else
									messagesList.toggleExpanded(delegateItem.tpMessage.row);
							}
						}
					} //headerWidget

					TPLabel {
						id: lblMessage
						text: delegateItem.tpMessage.text
						font: AppGlobals.smallFont
						visible: delegateItem.collapsed && text.length > 0
						singleLine: false
						width: delegateItem.widthAvailable

						anchors {
							top: headerWidget.bottom
							left: parent.left
							leftMargin: delegateItem.indentation
						}
						property bool _text_changed: false
						onTextChanged: {
							if (_text_changed)
								delegateItem.delegateHeight += height;
							_text_changed = true;
						}
						Component.onCompleted: delegateItem.delegateHeight += height;
					}

					Loader {
						id: actionsLoader
						asynchronous: true
						active: delegateItem.tpMessage.actionCount > 0
						visible: delegateItem.collapsed
						width: delegateItem.widthAvailable
						height: _place_holder_item !== null ? _place_holder_item._height : 0

						anchors {
							top: lblMessage.bottom
							topMargin: lblMessage.visible ? lblMessage.contentHeight : - AppSettings.itemSmallHeight
							leftMargin: delegateItem.indentation
						}

						property Item _place_holder_item: null

						sourceComponent: Item {
							id: actionsPlaceHolder

							property int _height: 0
							property int _row: 0
							property list<int> _row_width: [0]
							property list<Item> _items

							function addItem(item: Item, index: int, total_items: int): void {
								if (index === total_items - 1 && _row_width[_row] === 0) {
									item.anchors.horizontalCenter = horizontalCenter;
									if (index === 0)
										item.anchors.verticalCenter = verticalCenter;
									else
										item.anchors.top = _items[index-1].bottom;
									return;
								}
								if (item.width >= delegateItem.widthAvailable * 0.8) { //too big to shrink
									_row_width.push(0);
									++_row
									_height += item.height;
									if (index > 0)
										item.anchors.top = _items[index-1].bottom;
									else
										item.anchors.top = top;
									item.anchors.horizontalCenter = horizontalCenter;
								} else {
									if (item.width + _row_width[_row] <= delegateItem.widthAvailable * 0.9) { //this item fits on the current row
										if (_row_width[_row] === 0) {
											item.anchors.left = left;
											item.anchors.leftMargin = indicator.width;
											_height += item.height + 10;
											if (index > 0)
												item.anchors.top = _items[index-1].bottom;
											else
												item.anchors.top = top;
										} else {
											if (index > 0) {
												item.anchors.left = _items[index-1].right;
												item.anchors.top = _items[index-1].top;
											} else {
												item.anchors.verticalCenter = verticalCenter;
											}
										}
										_row_width[_row] = item.width;
									} else { //resize one or more items until they fit on row
										let row_width = 0;
										const prev_widget_width = index > 0 ? _items[index-1].width : 0;
										let shrink_prev = false;
										do { //resize either of the items at a time
											row_width = _row_width[_row];
											if (!shrink_prev) {
												if (item.width > delegateItem.widthAvailable * 0.5) //big, but shrinkable
													item.width *= 0.9; //shrink 10%
												shrink_prev = index > 0;
											} else { //shrink previous item
												row_width -= prev_widget_width;
												_items[index-1].width *= 0.9;
												row_width += _items[index-1].width;
												shrink_prev = false;
											}
											row_width += item.width;
										} while (row_width > delegateItem.widthAvailable * 0.95);
										_row_width[_row] = Math.ceil(row_width);
										if (index > 0) {
											item.anchors.left = _items[index-1].right;
											item.anchors.top = _items[index-1].top;
										} else {
											item.anchors.verticalCenter = verticalCenter;
										}

										if (_row_width[_row] >= delegateItem.widthAvailable * 0.9) {
											_row_width.push(0);
											++_row;
											_height += item.height;
										}
									}
								}
								item.anchors.margins = 5;
							}

							function setupActions(): void {
								for (let i = 0; i < delegateItem.tpMessage.actionCount; ++i) {
									let component, item;
									switch (delegateItem.tpMessage.actionType(i)) {
									case TPMessage.AT_BUTTON:
										component = Qt.createComponent("TpQml.Widgets", TPButton);
										item = component.createObject(actionsPlaceHolder, { text:
																	delegateItem.tpMessage.actionLabel(i) });
										break;
									case TPMessage.AT_CHECKBOX:
										component = Qt.createComponent("TpQml.Widgets", TPRadioButtonOrCheckBox);
										item = component.createObject(actionsPlaceHolder, { text:
											delegateItem.tpMessage.actionLabel(i), boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX});
										break;
									case TPMessage.AT_RADIO:
										component = Qt.createComponent("TpQml.Widgets", TPRadioButtonOrCheckBox);
										item = component.createObject(actionsPlaceHolder, { text:
											delegateItem.tpMessage.actionLabel(i), boxType: TPRadioButtonOrCheckBox.TP_RADIOBOX});
										break;
									case TPMessage.AT_NONE:
										continue;
									}
									if (item) {
										_items.push(item);
										actionsPlaceHolder.addItem(item, i, delegateItem.tpMessage.actionCount);
									}
								}
								delegateItem.delegateHeight += _height;
							}

							function clearActions(): void {
								for (let i = _items.length - 1; i >= 0; --i) {
									_items[i].destroy();
									_items.pop();
								}
								actionsPlaceHolder.children = 0;
								delegateItem.delegateHeight -= _height;
								_height = 0;
							}

							Connections {
								target: delegateItem.tpMessage
								function onActionsChanged(): void {
									actionsPlaceHolder.clearActions();
									actionsPlaceHolder.setupActions();
								}
								function onActionChanged(action_id: int): void {
									actionsPlaceHolder.childAt(action_id).text = delegateItem.tpMessage.actionLabel(action_id);
								}
							}
							Connections {
								target: onlineMsgsDlg
								function onPopupSizeChanged(w_ratio: real, h_ratio: real): void {
									for (let i = 0; i < _items.length; ++i)
										_items[i].width *= w_ratio;
								}
							}

							Component.onCompleted: {
								setupActions();
								actionsLoader._place_holder_item = this;
							}
						} //sourceComponent: GridLayout
					} //Loader: actionsLoader

					Loader {
					id: fileViewerLoader
					asynchronous: true
					visible: delegateItem.collapsed
					active: delegateItem.tpMessage.fileOps !== null
					width: _file_viewer !== null ? _file_viewer.minimumWidth : 0
					height: _file_viewer !== null ? _file_viewer.minimumHeight : 0

					anchors {
						top: delegateItem.tpMessage.actionCount > 0 ? actionsLoader.bottom : lblMessage.bottom
						topMargin: AppSettings.itemDefaultHeight
						horizontalCenter: parent.horizontalCenter
					}

					property TPFileViewer _file_viewer: null

					sourceComponent: TPFileViewer {
						fileOps: delegateItem.tpMessage.fileOps
						Component.onCompleted: {
							fileViewerLoader._file_viewer = this;
							delegateItem.delegateHeight += minimumHeight + (2 * AppSettings.itemDefaultHeight);
						}
					}
				}
			} //Rectangle: delegate's background
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
		visible: !onlineMsgsDlg._minimized
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
			visible: !onlineMsgsDlg._minimized
			anchors.centerIn: parent
			onClicked: mainLayout.currentIndex = mainLayout.currentIndex !== 2
														? 2 : (AppMessages.messagesModel.hasMessage ? 1 : 0);
		}
	} //Rectangle

	function openChat(user_idx: int): void {
		AppMessages.openChat(user_idx);
		mainLayout.currentIndex = 1;
	}

	function newMessage(user_idx: int): void {
		AppMessage.openNewMessageDialog(user_idx);
		mainLayout.currentIndex = 1;
	}
}
