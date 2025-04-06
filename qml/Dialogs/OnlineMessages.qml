import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

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
	visible: fullDialogVisible ? true : appMessages.count > 0

	property bool fullDialogVisible: false
	readonly property int dlgMaxWidth: appSettings.pageWidth * 0.8

	background: Rectangle {
		color: "lightblue"
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
			easing.type: Easing.InOutCubic
		}

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "height"
			to: mainIcon.height
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: {
			onlineMsgsDlg.x = appSettings.pageWidth - 80;
			onlineMsgsDlg.y = 180;
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
			easing.type: Easing.InOutCubic
		}

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "height"
			to: messagesList.height + topBar.height
			duration: 200
			easing.type: Easing.InOutCubic
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
			movableWidget: mainIcon
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
		height: messagesHeight > maxHeight ? maxHeight : messagesHeight
		visible: fullDialogVisible

		anchors {
			top: topBar.bottom
			left: parent.left
			right: parent.right
		}

		readonly property int maxHeight: appSettings.pageHeight*0.5
		readonly property int minimumMessageHeight: maxHeight*0.2
		property int messagesHeight: 0

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: messagesList.contentHeight > messagesList.height
		}
		ScrollBar.horizontal: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: messagesList.contentWidth > messagesList.width
		}

		delegate: SwipeDelegate {
			id: delegateItem
			swipe.enabled: !appMessages.messageEntry(index).sticky
			clip: true
			padding: 0
			spacing: 5
			width: messagesList.width

			property bool showActions: false

			contentItem: ColumnLayout {
				id: messageLayout
				spacing: 5
				opacity: 1 + swipe.position

				RowLayout {
					id: messageTextLayout
					height: 20
					Layout.fillWidth: true
					Layout.leftMargin: 5
					Layout.rightMargin: 5

					TPImage {
						source: appMessages.messageEntry(index).iconSource
						dropShadow: false
						width: 15
						height: 15
					}

					TPLabel {
						text: appMessages.messageEntry(index).date + "  " + appMessages.messageEntry(index).time
						font: AppGlobals.smallFont
						height: 15
						Layout.leftMargin: 20
					}

					TPButton {
						id: btnFoldIcon
						imageSource: delegateItem.showActions ? "fold-up.png" : "fold-down.png"
						hasDropShadow: false
						imageSize: 15
						Layout.alignment: Qt.AlignRight
						Layout.rightMargin: 5
						onClicked: delegateItem.showActions = !delegateItem.showActions;
					}
				}

				TPLabel {
					id: lblMessage
					text: appMessages.messageEntry(index).displayText
					color: "black"
					heightAvailable: messagesList.minimumMessageHeight
					elide: delegateItem.showActions ? Text.ElideNone : Text.ElideRight
					singleLine: !delegateItem.showActions
					Layout.leftMargin: 5
					Layout.rightMargin: 5
					width: onlineMsgsDlg.dlgMaxWidth - 10
					Layout.minimumWidth: width
					Layout.maximumWidth: width
					Layout.maximumHeight: _preferredHeight
					Layout.minimumHeight: _preferredHeight

					MouseArea {
						anchors.fill: parent
						onClicked: delegateItem.showActions = !delegateItem.showActions;
					}
				}

				GridLayout {
					id: actionsLayout
					columns: 3
					visible: delegateItem.showActions
					columnSpacing: 2
					rowSpacing: 5
					Layout.fillWidth: true
					Layout.leftMargin: 5
					Layout.rightMargin: 5

					Repeater {
						id: actionsRepeater
						model: appMessages.messageEntry(index).actions

						readonly property int msgIndex: index
						delegate: TPButton {
							required property int index
							text: appMessages.messageEntry(actionsRepeater.msgIndex).actions[index]
							onClicked: appMessages.messageEntry(actionsRepeater.msgIndex).execAction(index);
						}

						onItemAdded: (index, item) => {
							//items are added in reverse order(last to first)
							if (index === 0) {
								messagesList.messagesHeight += (model.length % 3 + 1)*30 + lblMessage.preferredHeight() + messageTextLayout.height;
								let i = 0;
								let rowWidth = 0;
								do {
									rowWidth += actionsRepeater.itemAt(i).width;
									if (i != 0 && i % 2 === 0) {
										console.log(i, actionsRepeater.itemAt(i-2).text, actionsRepeater.itemAt(i-2).Layout.leftMargin, rowWidth);
										actionsRepeater.itemAt(i-2).Layout.leftMargin = (dlgMaxWidth - rowWidth)/2;
										console.log(i, actionsRepeater.itemAt(i-2).text, actionsRepeater.itemAt(i-2).Layout.leftMargin);
										rowWidth = 0;
									}
								} while (++i < actionsRepeater.count);
								if (i % 2 !== 0) {
									if (--i % 2 !== 0)
										--i;
									actionsRepeater.itemAt(i).Layout.leftMargin = (dlgMaxWidth - rowWidth)/2;
								}
							}
						}
					}
				}
			} //ColumnLayout

			Rectangle {
				id: removeBackground
				anchors.fill: parent
				color: "lightgray"
				radius: 6
				layer.enabled: true
				visible: false
			}

			swipe.right: MultiEffect {
				id: removeRec
				anchors.fill: parent
				source: removeBackground
				shadowEnabled: true
				shadowOpacity: 0.5
				blurMax: 16
				shadowBlur: 1
				shadowHorizontalOffset: 5
				shadowVerticalOffset: 5
				shadowColor: "black"
				shadowScale: 1
				opacity: delegateItem.swipe.complete ? 0.8 : 0-delegateItem.swipe.position
				Behavior on opacity { NumberAnimation {} }

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

			Rectangle {
				id: backRec
				anchors.fill: parent
				radius: 6
				layer.enabled: true
				color: index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
				visible: false
			}

			background: MultiEffect {
				id: messageEffect
				visible: true
				source: backRec
				shadowEnabled: true
				shadowOpacity: 0.5
				blurMax: 16
				shadowBlur: 1
				shadowHorizontalOffset: 5
				shadowVerticalOffset: 5
				shadowColor: "black"
				shadowScale: 1
				opacity: 0.8
			}
		} //delegate
	} // messagesList
}
