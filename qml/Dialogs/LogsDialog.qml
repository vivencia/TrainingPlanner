import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

import "./LogsDialogComponents"

TPPopup {
	id: _dialog
	keepAbove: true
	useShape: true
	showTitleBar: true
	mouseItem: _minimized ? mainIcon : titleBar
	showBehavior: TPPopup.ALWAYS_VISIBLE
	defaultCoordinates: Qt.point((width - AppSettings.windowWidth) / 2, (height - AppSettings.windowHeight) / 2)
	normal_size: Qt.size(AppSettings.pageWidth * 0.8, AppSettings.pageWidth * 0.8)
	resizeable: true
	savePopupState: true

	TPLabel {
		text: qsTr("Application Logs")
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			rightMargin: _dialog.titleBar.titleBarButtons.width
			margins: 5
		}
	}

	TPScrollView {
		parentPage: ItemManager.appHomePage()
		enableNavButtons: false

		anchors {
			fill: parent
			topMargin: _dialog.titleBarHeight
		}

		LogsPane {
			id: messagesPane
			header: qsTr("Messages")
			icon: "messages"
			logsModel: ItemManager.messagesLog
			width: parent.width

			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}
		}
		LogsPane {
			id: corePane
			header: qsTr("Core")
			icon: "logs_"
			logsModel: ItemManager.coreLog
			width: parent.width

			anchors {
				top: messagesPane.bottom
				left: parent.left
				right: parent.right
			}
		}
		LogsPane {
			id: debugPane
			header: qsTr("Debug")
			icon: "logs_"
			logsModel: ItemManager.debugLog
			width: parent.width
			visible: ItemManager.debugLog !== null

			anchors {
				top: corePane.bottom
				left: parent.left
				right: parent.right
			}
		}
	}
}
