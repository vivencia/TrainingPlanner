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
	show_position: Qt.AlignBaseline
	defaultCoordinates: Qt.point(80, 180)
	normal_size: Qt.size(AppSettings.pageWidth * 0.8, AppSettings.pageWidth * 0.8)
	minimized_size: Qt.size(mainIcon.width, mainIcon.height)
	resizeable: true
	savePopupState: true
	show_minimize_button: false
	showCloseButton: false

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
			margins: 5
		}
	}

	TPLabel {
		id: topBar
		text: qsTr("Messages")
		visible: !onlineMsgsDlg._minimized
		horizontalAlignment: Text.AlignHCenter

		anchors {
			left: smallIcon.right
			verticalCenter: smallIcon.verticalCenter
			margins: 5
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
			Layout.fillHeight: true
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
				implicitHeight: 1.1 * headerWidget.height + (tpMessage.collapsed ? tpMessage.delegateHeight : 0.0)
				//forces an update of contentItem or, in this case, TPBackRec. Otherwise, the delegate gets its new size,
				//but the background does not follow it
				height: implicitHeight

				required property TPMessage tpMessage
				required property bool expanded
				required property int index
				readonly property bool hasChildren: tpMessage.childCount > 0
				readonly property int indentation: -10 + tpMessage.depth * 12
				readonly property int widthAvailable: width - indentation - 5
				property real delegateHeight: 0.0

				/**	Because of messagesList.forceLayout(), some delegate properties must be kept on tpMessage. forceLayout()
					destroys all the delegates and, therefore, all of its properties are reset to the defaults, loosing the
					current values. TreeView keeps record only of the initial delegate's implicitHeight, without forceLayout(),
					once the view is updated with a new message or some message is collapsed changed, the items might get clobbered or
					a gap the size of the collapsed part appears. Also, to speed the creation of delegates, several parts
					are managed on c++ and created only once when needed. TreeView is very limited indeed.
				**/
				Connections {
					target: delegateItem.tpMessage
					function onCollapsedChanged(): void {
						messagesList.forceLayout(); //force a repositioning of all the visible items
					}
				}

				TPBackRec {
					radius: 8
					opacity: 0.8
					enableShadow: true
					backColor: {
						if (delegateItem.tpMessage.type === TPMessage.MT_PHANTON) return "transparent";
						let _color = delegateItem.tpMessage.row % 2 !== 0 ? AppSettings.primaryDarkColor : AppSettings.primaryColor;
						if (delegateItem.tpMessage.depth > 0)
							_color = Qt.lighter(_color, delegateItem.tpMessage.depth * 1.2 + (delegateItem.tpMessage.depth * 0.1));
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
							source: delegateItem.tpMessage.collapsed ? "fold-up.png" : "fold-down.png"
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
									delegateItem.tpMessage.collapsed = !delegateItem.tpMessage.collapsed;
								else
									messagesList.toggleExpanded(delegateItem.tpMessage.row);
							}
						}
					} //headerWidget

					TPLabel {
						id: lblMessage
						text: delegateItem.tpMessage.text
						font: AppGlobals.smallFont
						visible: delegateItem.tpMessage.collapsed && text.length > 0
						singleLine: false
						width: delegateItem.widthAvailable

						anchors {
							top: headerWidget.bottom
							left: parent.left
							leftMargin: delegateItem.indentation
						}
						Component.onCompleted: {
							if (text.length > 0)
								delegateItem.tpMessage.setMessageComponentHeight(TPMessage.MC_TEXT, height);
						}
					}

					Item {
						id: actionsPlaceHolder
						width: delegateItem.widthAvailable
						visible: delegateItem.tpMessage.collapsed

						anchors {
							top: lblMessage.bottom
							topMargin: lblMessage.visible ? lblMessage.contentHeight : -20
							leftMargin: delegateItem.indentation
							left: parent.left
							right: parent.right
						}
						Component.onCompleted: delegateItem.tpMessage.setActionsLayoutParent(this);
					}

					Item {
						visible: delegateItem.tpMessage.collapsed

						anchors {
							top: actionsPlaceHolder.bottom
							topMargin: AppSettings.itemDefaultHeight
							left: parent.left
							leftMargin: delegateItem.indentation
							right: parent.right
						}
						Component.onCompleted: delegateItem.tpMessage.setFileViewerParent(this);
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

				onItemSelected: (userIdx) => newChatOrMessagePane._useridx = userIdx;
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

				TPButton2 {
					text: qsTr("Chat")
					sourceImage: "chat.png"
					enabled: newChatOrMessagePane._useridx > 0
					Layout.preferredWidth: preferredWidth
					Layout.maximumWidth: parent.width / 2
					onClicked: {
						AppMessages.openChat(newChatOrMessagePane._useridx);
						mainLayout.currentIndex = 1;
					}
				}
				TPButton2 {
					text: qsTr("Send message")
					sourceImage: "send-message.png"
					enabled: newChatOrMessagePane._useridx > 0
					Layout.preferredWidth: preferredWidth
					Layout.maximumWidth: parent.width / 2
					onClicked: {
						AppMessages.openNewMessageDialog(newChatOrMessagePane._useridx);
						mainLayout.currentIndex = 1;
					}
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

		TPButton2 {
			imageSource: mainLayout.currentIndex !== 2 ? "add-new.png" : "revert.png"
			width: AppSettings.itemDefaultHeight
			height: width
			visible: !onlineMsgsDlg._minimized
			anchors.centerIn: parent
			onClicked: mainLayout.currentIndex = mainLayout.currentIndex !== 2
														? 2 : (AppMessages.messagesModel.hasMessage ? 1 : 0);
		}
	} //Rectangle
}
