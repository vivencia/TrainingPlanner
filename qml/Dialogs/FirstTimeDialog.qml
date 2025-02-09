import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"
import "../User"
import ".."

TPPopup {
	id: firstTimeDlg
	objectName: "firstTimerDlg"
	modal: true
	keepAbove: true
	closeButtonVisible: false
	width: appSettings.pageWidth - 50
	height: moduleHeight + frmFooter.height
	x: (appSettings.pageWidth - width)/2 // horizontally centered
	finalYPos: (appSettings.pageHeight - height)/2 // vertically centered

	readonly property int moduleHeight: usrProfile.height

	StackLayout {
		id: stackLayout
		currentIndex: appTr.translatorOK() ? 1 : 0

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			bottom: frmFooter.top
			margins: 5
		}

		UserLanguage {
			Layout.fillWidth: true
			Layout.fillHeight: true
		}

		UserWelcome {
			Layout.fillWidth: true
			Layout.fillHeight: true
		}

		UserExistingFromNet {
			Layout.fillWidth: true
			Layout.fillHeight: true
		}

		UserPersonalData {
			userRow: 0
			parentPage: firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.fillHeight: true
		}

		UserContact {
			userRow: 0
			Layout.fillWidth: true
			Layout.fillHeight: true
		}

		UserCoach {
			userRow: 0
			Layout.fillWidth: true
			Layout.fillHeight: true
		}

		UserProfile {
			id: usrProfile
			userRow: 0
			parentPage: firstTimeDlg.parentPage
			width: firstTimeDlg.width - 20
		}

		UserReady {
			Layout.fillWidth: true
			Layout.fillHeight: true
		}
	}

	Frame {
		id: frmFooter
		height: 30
		background: Rectangle {
			border.width: 0
			radius: 8
		}

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		TPButton {
			id: btnPrev
			text: qsTr("Previous")
			imageSource: "back.png"
			hasDropShadow: false
			leftAlign: false
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
			text: qsTr("Next")
			imageSource: "next.png"
			hasDropShadow: false
			enabled: stackLayout.currentIndex < stackLayout.count ? stackLayout.itemAt(stackLayout.currentIndex).bReady : false

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				if (stackLayout.currentIndex === stackLayout.count - 1) {
					userModel.mainUserConfigurationFinished();
					closePopup();
				}
				else {
					if (stackLayout.currentIndex === 2)
						//Might be trying to connect online to retrieve existing user info. But, if Next was clicked, it means that a local
						//new user will be created, so we must cancel pending requests to try to retrieve info from the net
						userModel.cancelPendingOnlineRequests();
					else {
						if (stackLayout.currentIndex >= 3 && stackLayout.currentIndex < stackLayout.count - 2)
							stackLayout.itemAt(stackLayout.currentIndex+1).focusOnFirstField();
					}
				}
				stackLayout.currentIndex++;
			}
		}
	}
}
