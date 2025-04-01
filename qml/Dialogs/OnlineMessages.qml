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
	closePolicy: Popup.CloseOnPressOutside
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	visible: fullDialogVisible ? true : appMessages.count > 0

	property bool fullDialogVisible: false

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
	}
	ParallelAnimation
	{
		id: expand
		alwaysRunToEnd: true

		PropertyAnimation {
			target: onlineMsgsDlg
			property: "width"
			to: appSettings.pageWidth * 0.8
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
	}

	TPImage {
		id: mainIcon
		source: "messages"
		dropShadow: false
		width: 100
		height: 100
		visible: !fullDialogVisible

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

		TPLabel {
			id: topBarText
			text: qsTr("Messages")

			anchors {
				horizontalCenter: parent.horizontalCenter;
				horizontalCenterOffset: - smallIcon.width/2
				verticalCenter: parent.horizontalCenter;
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
				verticalCenter: parent.horizontalCenter;
			}
		}

		TPMouseArea {
			movingWidget: topBar
			movableWidget: onlineMsgsDlg

			onPressed: (mouse) => pressedFunction(mouse);
			onPositionChanged: (mouse) => positionChangedFunction(mouse);
			onMouseClicked: {
				fullDialogVisible = false;
				shrink.start();
			}
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
			active: true; visible: pendingCoachesList.contentHeight > pendingCoachesList.height
		}
		ScrollBar.horizontal: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: pendingCoachesList.contentWidth > pendingCoachesList.width
		}

		delegate: SwipeDelegate {
			id: delegateItem
			swipe.enabled: !appMessages.messageEntry(index).sticky
			clip: true
			padding: 5
			spacing: 5
			implicitHeight: index === messagesList.currentIndex ? (appMessages.messageEntry(index).hasActions ? messageLayout.height : messagesList.minimumMessageHeight)
								: lblMessage.height

			property bool showActions: false

			ColumnLayout {
				id: messageLayout
				spacing: 5
				opacity: 1 + swipe.position
				anchors {
					fill: parent
					margins: 5
				}

				Row {
					height: 15
					Layout.fillWidth: true

					TPImage {
						source: appMessages.messageEntry(index).iconSource
						dropShadow: false
						width: 15
						height: 15
					}

					TPLabel {
						text: appMessages.messageEntry(index).date + "  " + appMessages.messageEntry(index).time
						fontSizeMode: Text.Fit
						height: 15
						Layout.leftMargin: 20
					}

					TPButton {
						id: btnFoldIcon
						imageSource: showActions ? "fold-up" : "fold-down"
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
					heightAvailable: minimumMessageHeight
					widthAvailable: parent.width - 10
					wrapMode: delegateItem.showActions ? Text.NoWrap : Text.WordWrap
					singleLine: !delegateItem.showActions
					Layout.fillWidth: true
					Layout.leftMargin: 5
					Layout.rightMargin: 5

					MouseArea {
						anchors.fill: parent
						onClicked: delegateItem.showActions = !delegateItem.showActions;
					}
				}

				Repeater {
					id: actionsRepeater
					model: appMessages.messageEntry(index).acions
					Layout.fillWidth: true
					Layout.leftMargin: 5
					Layout.rightMargin: 5

					property int delegateIndex: index

					delegate: GridLayout {
						columns: 3
						required property int index

						TPButton {
							text: appMessages.messageEntry(actionsRepeater.delegateIndex).acions[index]
							onClicked: appMessages.messageEntry(actionsRepeater.delegateIndex).execAction(index);
						}
					}
				}
			}

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

			contentItem: ColumnLayout {

			}

			onClicked: {
				curRow = userModel.findUserByName(userModel.coachesNames[index]);
				userModel.currentRow = curRow;
				coachesList.currentIndex = index;
			}
		}
	}
}
