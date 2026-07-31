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

		TreeView {
			id: messagesList
			model: AppMessages
			clip: true
			Layout.fillWidth: true
			Layout.fillHeight: true

			selectionModel: ItemSelectionModel {}

			delegate: TreeViewDelegate {
				id: delegateItem

				readonly property real _padding: 5
				readonly property real szHeight: contentItem.implicitHeight * 2.5
				required property TPMessage message
				property bool collapsed: false

				background: Rectangle { // Background rectangle enabled to show the alternative row colors
					id: background
					opacity: 0.8

					color: {
						if (delegateItem.model.row === delegateItem.treeView.currentRow) {
							return Qt.lighter(palette.highlight, 1.2)
						} else {
							if (delegateItem.treeView.alternatingRows && delegateItem.model.row % 2 !== 0) {
								return (Application.styleHints.colorScheme === Qt.Light) ?
										 Qt.darker(palette.alternateBase, 1.25) :
										 Qt.lighter(palette.alternateBase, 2.)
							} else {
							   return palette.base
							}
						}
					}
					Rectangle { // The selection indicator shown on the left side of the highlighted row
						width: delegateItem._padding
						height: parent.height
						visible: !delegateItem.model.column
						color: {
							if (delegateItem.model.row === delegateItem.treeView.currentRow) {
								return (Application.styleHints.colorScheme === Qt.Light) ?
										 Qt.darker(palette.highlight, 1.25) :
										 Qt.lighter(palette.highlight, 2.)
							} else {
								return "transparent"
							}
						}
					}
				}

				indicator: Item {
					x: delegateItem._padding + delegateItem.depth * delegateItem.indentation
					implicitWidth: delegateItem.szHeight
					implicitHeight: delegateItem.szHeight
					visible: delegateItem.isTreeNode && delegateItem.hasChildren
					rotation: delegateItem.expanded ? 90 : 0

					TapHandler {
						onSingleTapped: {
							const index = delegateItem.treeView.index(delegateItem.model.row, delegateItem.model.column)
							delegateItem.treeView.selectionModel.setCurrentIndex(index, ItemSelectionModel.NoUpdate)
							delegateItem.treeView.toggleExpanded(delegateItem.model.row)
						}
					}
					TPImage {
						source: "arrow_icon.png"
						width: parent.width / 3
						height: parent.height / 3
						anchors.centerIn: parent
					}
				}

				contentItem: ColumnLayout {
					id: messageLayout
					spacing: 5
					x: delegateItem._padding + (delegateItem.depth + 1 * delegateItem.indentation)
					width: parent.width - delegateItem._padding - x

					Item {
						Layout.preferredWidth: parent.width - 10
						Layout.preferredHeight: AppSettings.itemExtraLargeHeight
						Layout.leftMargin: 5
						Layout.rightMargin: 5

						TPImage {
							id: msgImage
							source: delegateItem.message.icon
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
							text: delegateItem.message.title + "<br>" + delegateItem.message.dateTime
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
							source: delegateItem.message.extraImage
							visible: delegateItem.message.extraImage.length > 0
							width: visible ? AppSettings.itemSmallHeight : 0
							height: visible ? AppSettings.itemSmallHeight : 0

							anchors {
								verticalCenter: parent.verticalCenter
								left: lblTitle.right
							}

							TPLabel {
								text: delegateItem.message.extraInfo
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
							dropShadow: false
							width: AppSettings.itemSmallHeight
							height: AppSettings.itemSmallHeight

							anchors {
								verticalCenter: parent.verticalCenter
								left: lblTitle.right
							}
						}

						MouseArea {
							anchors.fill: parent
							onClicked: delegateItem.collapsed = !delegateItem.collapsed;
						}
					}

					TPLabel {
						id: lblMessage
						text: delegateItem.message.text
						font: AppGlobals.smallFont
						visible: delegateItem.collapsed
						singleLine: false
						Layout.fillWidth: true
						Layout.leftMargin: 10
						Layout.rightMargin: 10
						Component.onCompleted: msgTextLoader._label = this;
					}

					Loader {
						id: fileViewerLoader
						asynchronous: true
						active: delegateItem.message.fileOps !== null
						visible: delegateItem.collapsed
						Layout.preferredWidth: active ? Math.max(200, _file_viewer.minimumWidth) : 0
						Layout.preferredHeight: active ? Math.max(200, _file_viewer.minimumHeight) : 0
						Layout.alignment: Qt.AlignHCenter

						property TPFileViewer _file_viewer
						sourceComponent: TPFileViewer {
							fileOps: delegateItem.message.fileOps
							Component.onCompleted: fileViewerLoader._file_viewer = this;
						}
					}

					Loader {
						id: actionsLoader
						asynchronous: true
						active: delegateItem.message.actionCount > 0
						visible: delegateItem.collapsed
						Layout.fillWidth: true
						Layout.leftMargin: 5
						Layout.rightMargin: 5
						Layout.preferredHeight: active ? _layout.childrenRect.height : 0

						property GridLayout _layout

						onActiveChanged: {
							if (active) {
								for (let i = 0; i < delegateItem.message.actionCount; ++i) {
									let component, item;
									switch (delegateItem.message.actionType()) {
									case TPMessage.AT_BUTTON:
										component = Qt.createComponent("TpQml.Widgets", TPButton, { text:
																		   delegateItem.message.actionLabel(index) });
										item = component.createObject(actionsLayout, {});
										break;
									case TPMessage.AT_CHECKBOX:
										component = Qt.createComponent("TpQml.Widgets", TPRadioButtonOrCheckBox, { text:
											delegateItem.message.actionLabel(index), boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX});
										item = component.createObject(actionsLayout, {});
										break;
									case TPMessage.AT_RADIO:
										component = Qt.createComponent("TpQml.Widgets", TPRadioButtonOrCheckBox, { text:
											delegateItem.message.actionLabel(index), boxType: TPRadioButtonOrCheckBox.TP_RADIOBOX});
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
				listConfirmed: true
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
