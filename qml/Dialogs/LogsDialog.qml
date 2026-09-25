import QtQuick

import TpQml
import TpQml.Pages
import TpQml.Widgets

import "./LogsDialogComponents"

TPPopup {
	objectName: "LogsDialog"
	id: _dialog
	keepAbove: true
	useShape: true
	showTitleBar: true
	showBehavior: TPPopup.ALWAYS_VISIBLE
	show_position: Qt.AlignBaseline
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
		parentPage: ItemManager.appHomePage() as TPPage
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
			visible: ItemManager.debugLog !== null

			anchors {
				top: corePane.bottom
				left: parent.left
				right: parent.right
			}
		}
	}
}
