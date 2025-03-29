import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
	width: appSettings.pageWidth * 0.8
	height: messagesList.height

	property bool fullDialogVisible: false

	SequentialAnimation {
		id: anim
		alwaysRunToEnd: true

		// Expand the button
		PropertyAnimation {
			target: control
			property: "scale"
			to: 1.5
			duration: 200
			easing.type: Easing.InOutCubic
		}

		// Shrink back to normal
		PropertyAnimation {
			target: control
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: clicked();
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

			onMouseClicked: {
				fullDialogVisible = true;
				anim.start();
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

			onMouseClicked: {
				fullDialogVisible = false;
				anim.start();
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

		property int messagesHeight: 0
		readonly property int maxHeight: appSettings.pageHeight*0.5
	}
}
