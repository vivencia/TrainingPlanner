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
	height: 210
	x: (windowWidth - width) / 2 // horizontally centered
	y: (windowHeight - height) / 2 // vertically centered

	StackLayout {
		id: stackLayout

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			bottom: frmFooter.top
		}

		UserPersonalData {

		}
	}

	Frame {
		id: frmFooter
		height: 30

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		TPButton {
			id: btnPrev
			text: qsTr("Previous")
			imageSource: "back.png"
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
			enabled: stackLayout.currentIndex < stackLayout.count

			anchors {
				right: parent.right
				rightMargin: 20
				verticalCenter: parent.verticalCenter
			}

			onClicked: stackLayout.currentIndex++;
		}
	}
}
