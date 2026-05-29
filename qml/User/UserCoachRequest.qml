pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
	id: dlgCoachRequest
	keepAbove: true
	showTitleBar: true
	closeButtonVisible: true
	open_in_window: true
	width: AppSettings.pageWidth - 20
	height: AppSettings.pageHeight * 0.4

	onOpened: AppUserModel.getOnlineCoachesList();

	TPLabel {
		id: lblTitle
		text: qsTr("Available coaches online")
		height: AppSettings.itemDefaultHeight

		anchors {
			top: dlgCoachRequest.titleBar.top
			left: dlgCoachRequest.titleBar.left
			right: dlgCoachRequest.titleBar.right
			margins: 5
			rightMargin: dlgCoachRequest.btnClose.width
		}
	}

	ColumnLayout {
		spacing: 5

		anchors {
			top: lblTitle.bottom
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			margins: 5
		}

		TPLabel {
			text: qsTr("No coaches available")
			font: AppGlobals.largeFont
			horizontalAlignment: Text.AlignHCenter
			visible: !availableCoachesList.visible
			Layout.maximumWidth: parent.width - 10
		}

		TPCoachesAndClientsList {
			id: availableCoachesList
			listAvailable: true
			listCoaches: true
			multipleSelection: true
			buttonString: qsTr("Send request to the selected coaches")
			visible: model.count > 0
			Layout.fillWidth: true
			Layout.fillHeight: true

			onButtonClicked: AppUserModel.sendRequestToCoaches();
			onItemButtonClicked: (userIdx) => viewResumeLoader

			Loader {
				id: viewResumeLoader
				active: false
				asynchronous: true
				property TPFileViewer _file_viewer

				sourceComponent: TPFileViewer {
					missingFileInfo: qsTr(`The coach's resumè file could not be found.
						You can try to download it by pressing the second button from the left on the bottom of the screen`)
					attemptToGetFile: true
					onWindowStateChanged: (window_state) => {
						if (window_state === TPFileViewer.WS_NORMAL)
							viewResumeLoader.active = false;
					}
					Component.onCompleted: viewResumeLoader._file_viewer = this;
				}
			}
			function viewResume(user_idx: int): void {
				viewResumeLoader._file_viewer.fileName = AppUserModel.resume(user_idx);
				viewResumeLoader._file_viewer.startFullScreen();
			}
		} //ListView
	} //ColumnLayout
}
