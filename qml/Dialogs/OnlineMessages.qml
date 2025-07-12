import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Popup {
	id: onlineMsgsDlg
	objectName: "TPPopup"
	closePolicy: Popup.NoAutoClose
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	x: appSettings.pageWidth - 80
	y: 180
	width: mainIcon.width
	height: mainIcon.height
	visible: appMessages.count > 0

	property bool fullDialogVisible: false
	readonly property int dlgMaxWidth: appSettings.pageWidth * 0.8

	property int msgCount: appMessages.count
	onMsgCountChanged: {
		console.log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  ", msgCount);
	}

	background: Rectangle {
		color: "transparent"
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
			onlineMsgsDlg.x = onlineMsgsDlg.width;
			if (onlineMsgsDlg.x + mainIcon.width > appSettings.pageWidth)
				onlineMsgsDlg.x -= appSettings.pageWidth - 80;
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
			to: messagesList.height + topBar.height
			duration: 200
			easing.type: Easing.OutQuad
		}

		onFinished: {
			onlineMsgsDlg.x = appSettings.pageWidth - onlineMsgsDlg.width - 10;
			if (onlineMsgsDlg.y + onlineMsgsDlg.height > appSettings.pageHeight)
				onlineMsgsDlg.y = appSettings.pageHeight - onlineMsgsDlg.height;
		}
	}

	TPImage {
		id: mainIcon
		source: "messages"
		width: 50
		height: 50
		visible: !fullDialogVisible
		anchors {
			verticalCenter: parent.verticalCenter
			horizontalCenter: parent.horizontalCenter
		}

		TPMouseArea {
			movingWidget: mainIcon
			movableWidget: onlineMsgsDlg
			enabled: !fullDialogVisible

			onPressed: (mouse) => pressedFunction(mouse);
			onPositionChanged: (mouse) => positionChangedFunction(mouse);
			onMouseClicked: {
				fullDialogVisible = true;
				expand.start();
			}
		}
	}

	TPToolBar {
		id: topBar
		visible: fullDialogVisible
		height: 30

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		TPLabel {
			id: topBarText
			text: qsTr("Messages")

			anchors {
				horizontalCenter: parent.horizontalCenter;
				horizontalCenterOffset: 0 - smallIcon.width/2
				verticalCenter: parent.verticalCenter;
			}
		}

		TPImage {
			id: smallIcon
			source: "messages"
			dropShadow: false
			width: 25
			height: 25

			anchors {
				left: topBarText.right
				verticalCenter: parent.verticalCenter;
			}
		}

		TPMouseArea {
			movingWidget: parent
			movableWidget: onlineMsgsDlg

			onPressed: (mouse) => pressedFunction(mouse);
			onPositionChanged: (mouse) => positionChangedFunction(mouse);
			onMouseClicked: shrink.start();
		}
	}

	ListView {
		id: messagesList
		contentHeight: availableHeight
		contentWidth: availableWidth
		spacing: 5
		clip: true
		model: appMessages
		height: maxHeight
		visible: fullDialogVisible

		anchors {
			top: topBar.bottom
			left: parent.left
			right: parent.right
		}

		readonly property int maxHeight: appSettings.pageHeight*0.5
		property int messagesHeight: 0

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: messagesList.contentHeight > messagesList.height
		}

		delegate: SwipeDelegate {
			id: delegateItem
			swipe.enabled: !appMessages.messageEntry(index).sticky
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
				id: backRec
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
							source: appMessages.messageEntry(index).iconSource
							dropShadow: false
							width: 20
							height: 20
						}

						TPLabel {
							text: appMessages.messageEntry(index).date + "  " + appMessages.messageEntry(index).time
							font: AppGlobals.smallFont
							height: 15
							Layout.leftMargin: 20
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
					text: appMessages.messageEntry(index).displayText
					color: "black"
					elide: delegateItem.showActions ? Text.ElideNone : Text.ElideRight
					wrapMode: delegateItem.showActions ? Text.WordWrap : Text.NoWrap
					singleLine: !delegateItem.showActions
					width: onlineMsgsDlg.dlgMaxWidth - 10
					height: delegateItem.showActions ? preferredHeight() : appSettings.itemDefaultHeight
					Layout.leftMargin: 20

					MouseArea {
						anchors.fill: parent
						onClicked: delegateItem.setShowActions(!delegateItem.showActions);
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

					Repeater {
						id: actionsRepeater
						model: appMessages.messageEntry(index).actions

						readonly property int msgIndex: index
						delegate: TPButton {
							text: appMessages.messageEntry(actionsRepeater.msgIndex).actions[index]
							width: constrainSize ? actionsLayout.maxButtonWidth : defaultWidth()
							autoSize: constrainSize
							Layout.leftMargin: 0
							Layout.rightMargin: 0
							onClicked: appMessages.messageEntry(actionsRepeater.msgIndex).execAction(index);

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
			swipe.onCompleted: appMessages.removeMessage(appMessages.messageEntry(index));
		} //delegate
	} // messagesList
}
