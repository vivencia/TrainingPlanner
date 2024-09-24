import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"
import "../User"

TPPopup {
	id: firstTimeDlg
	objectName: "firstTimerDlg"
	bKeepAbove: true
	modal: true
	width: windowWidth - 50
	height: moduleHeight + frmFooter.height
	x: (windowWidth - width)/2 // horizontally centered
	finalYPos: (windowHeight - height)/2 // vertically centered

	readonly property int moduleHeight: usrContact.moduleHeight

	StackLayout {
		id: stackLayout
		currentIndex: appTr.translatorOK() ? 1 : 0

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			bottom: frmFooter.top
		}

		UserLanguage {
			width: firstTimeDlg.width - 20
			height: moduleHeight
		}

		UserWelcome {
			width: firstTimeDlg.width - 20
			height: moduleHeight
		}

		UserPersonalData {
			userRow: 0
			parentPage: firstTimeDlg.parentPage
			width: firstTimeDlg.width - 20
			height: moduleHeight
		}

		UserContact {
			id: usrContact
			userRow: 0
			width: firstTimeDlg.width - 20
		}

		UserCoach {
			userRow: 0
			width: firstTimeDlg.width - 20
			height: moduleHeight
		}

		UserProfile {
			userRow: 0
			parentPage: firstTimeDlg.parentPage
			width: firstTimeDlg.width - 20
			height: moduleHeight
		}

		UserReady {
			width: firstTimeDlg.width - 20
			height: moduleHeight
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
					mainwindow.bBackButtonEnabled = true;
					mainwindow.mainWindowStarted();
					close();
				}
				else {
					if (stackLayout.currentIndex >= 2)
						appDB.saveUser(0);
					if (stackLayout.currentIndex >= 1)
						stackLayout.itemAt(stackLayout.currentIndex+1).focusOnFirstField();
				}
				stackLayout.currentIndex++;
			}
		}
	}
}
