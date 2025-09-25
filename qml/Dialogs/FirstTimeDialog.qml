import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"
import "../User"
import ".."

TPPopup {
	id: firstTimeDlg
	modal: true
	keepAbove: true
	showTitleBar: false
	closeButtonVisible: false
	width: appSettings.pageWidth - 50
	height: stackLayout.childrenRect.height
	x: (appSettings.pageWidth - width)/2 // horizontally centered
	finalYPos: (appSettings.pageHeight - height)/2 // vertically centered

	readonly property int minimumHeight: appSettings.windowHeight * 0.5
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
			Layout.preferredHeight: minimumHeight
		}

		UserWelcome {
			Layout.fillWidth: true
			Layout.preferredHeight: minimumHeight
		}

		UserExistingFromNet {
			Layout.fillWidth: true
			Layout.minimumHeight: minimumHeight
		}

		UserPersonalData {
			userRow: 0
			parentPage: firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: minimumHeight
		}

		UserContact {
			userRow: 0
			Layout.fillWidth: true
			Layout.minimumHeight: minimumHeight
		}

		UserCoach {
			userRow: 0
			parentPage: firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: minimumHeight
		}

		UserProfile {
			id: usrProfile
			userRow: 0
			parentPage: firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: minimumHeight
		}

		UserReady {
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.preferredHeight: minimumHeight
		}

		Component.onCompleted: currentIndex = appSettings.userLocale.length > 0 ? 1 : 0;
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
					finish();
				else {
					if (stackLayout.currentIndex === 2) {
						//Might be trying to connect online to retrieve existing user info. But, if Next was clicked, it means that a local
						//new user will be created, so we must cancel pending requests to try to retrieve info from the net
						userModel.cancelPendingOnlineRequests();
					}
				}
				if (nextStartsTheApp)
					finish();
				else {
					stackLayout.currentIndex++;
					if (stackLayout.currentIndex >= 3 && stackLayout.currentIndex < stackLayout.count - 2)
						stackLayout.itemAt(stackLayout.currentIndex).focusOnFirstField();
				}
			}
		}
	}

	function finish(): void {
		userModel.setMainUserConfigurationFinished();
		closePopup();
	}
}
