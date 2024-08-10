import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"
import "../User"

TPPopup {
	id: firstTimeDlg
	bKeepAbove: true
	modal: true
	width: windowWidth - 50
	height: moduleHeight
	x: (windowWidth - width)/2 // horizontally centered
	finalYPos: (windowHeight - height)/2 // vertically centered

	readonly property int moduleHeight: usrData.implicitHeight + frmFooter.height

	StackLayout {
		id: stackLayout

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			bottom: frmFooter.top
		}

		UserWelcome {
			width: firstTimeDlg.width - 20
			height: moduleHeight
		}

		UserPersonalData {
			id: usrData
		}

		UserContact {
			width: firstTimeDlg.width - 20
			height: moduleHeight
		}

		UserProfile {
			width: firstTimeDlg.width - 20
			height: moduleHeight
			parentPage: firstTimeDlg.parentPage
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
			enabled: stackLayout.currentIndex < stackLayout.count ? stackLayout.itemAt(stackLayout.currentIndex).bReady : false

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				if (stackLayout.currentIndex === stackLayout.count - 1) {
					mainwindow.checkInitialArguments();
					mainwindow.bBackButtonEnabled = true;
					close();
				}
				else {
					appDB.saveUser();
					stackLayout.itemAt(stackLayout.currentIndex+1).focusOnFirstField();
				}
				stackLayout.currentIndex++;
			}
		}
	}
}
