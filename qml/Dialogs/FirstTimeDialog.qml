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

		UserProfile {
			width: firstTimeDlg.width - 20
			height: moduleHeight
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
			//enabled: stackLayout.currentIndex < stackLayout.count ? stackLayout.itemAt(stackLayout.currentIndex).bReady : false
			enabled: stackLayout.currentIndex < stackLayout.count

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}

			onClicked: stackLayout.currentIndex++;
		}
	}
}
