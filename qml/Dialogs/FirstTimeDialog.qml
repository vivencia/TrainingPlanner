import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.User
import TpQml.Widgets

TPPopup {
	id: _firstTimeDlg
	modal: true
	keepAbove: true
	showTitleBar: false
	closeButtonVisible: false
	width: AppSettings.pageWidth - 50
	height: stackLayout.childrenRect.height
	x: (AppSettings.pageWidth - width)/2 // horizontally centered
	finalYPos: (AppSettings.pageHeight - height)/2 // vertically centered

	readonly property int minimumHeight: AppSettings.windowHeight * 0.5
	property bool nextStartsTheApp: false

	onBackKeyPressed: {
		if (stackLayout.currentIndex > 0)
			stackLayout.currentIndex--;
	}

	StackLayout {
		id: stackLayout

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			margins: 5
		}

		UserLanguage {
			Layout.fillWidth: true
			Layout.preferredHeight: _firstTimeDlg.minimumHeight
		}

		UserWelcome {
			Layout.fillWidth: true
			Layout.preferredHeight: _firstTimeDlg.minimumHeight
		}

		UserExistingFromNet {
			parentDialog: _firstTimeDlg
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserPersonalData {
			userRow: 0
			parentPage: _firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserContact {
			userRow: 0
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserCoach {
			userRow: 0
			parentPage: _firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserProfile {
			id: usrProfile
			userRow: 0
			parentPage: _firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserReady {
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.preferredHeight:_firstTimeDlg. minimumHeight
		}

		Component.onCompleted: currentIndex = AppSettings.userLocale.length > 0 ? 1 : 0;
	}

	Frame {
		id: frmFooter
		height: btnPrev.height + 10
		background: Rectangle {
			border.width: 0
			radius: 8
		}

		anchors {
			left: parent.left
			right: parent.right
			top: stackLayout.bottom
		}

		TPButton {
			id: btnPrev
			text: qsTr("Previous")
			imageSource: "back.png"
			hasDropShadow: false
			iconOnTheLeft: true
			autoSize: true
			enabled: stackLayout.currentIndex > 0

			anchors {
				right: btnNext.left
				rightMargin: 20
				verticalCenter: parent.verticalCenter
			}

			onClicked: stackLayout.currentIndex--;
		}

		TPButton {
			id: btnNext
			text: stackLayout.currentIndex < stackLayout.count - 1 ? qsTr("Next") : qsTr("Conclude")
			imageSource: "next.png"
			hasDropShadow: false
			autoSize: true
			enabled: stackLayout.currentIndex < stackLayout.count ? stackLayout.itemAt(stackLayout.currentIndex).bReady : false

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				if (stackLayout.currentIndex === stackLayout.count - 1)
					_firstTimeDlg.finish();
				if (_firstTimeDlg.nextStartsTheApp)
					_firstTimeDlg.finish();
				else {
					stackLayout.currentIndex++;
					if (stackLayout.currentIndex >= 3 && stackLayout.currentIndex < stackLayout.count - 2)
						stackLayout.itemAt(stackLayout.currentIndex).focusOnFirstField();
				}
			}
		}
	}

	function finish(): void {
		AppUserModel.setMainUserConfigurationFinished();
		closePopup();
	}
}
