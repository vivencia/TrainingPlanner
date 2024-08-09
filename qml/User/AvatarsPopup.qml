import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "../TPWidgets"
import ".."

TPPopup {
	id: avatarsDlg
	bKeepAbove: false
	width: windowWidth
	height: windowWidth + 30
	x: 0
	finalYPos: targetImageItem.y + 10

	required property Item targetImageItem
	required property Item callerWidget

	Rectangle {
		id: footerBar
		width: windowWidth
		height: 30
		color: "transparent"

		Label {
			id: lblChooseImage
			text: qsTr("Choose another image...")
			color: AppSettings.fontColor
			font.pointSize: AppSettings.fontSizeText
			font.bold: true
			height: 25
			topPadding: 5
			leftPadding: 10
			width: parent.width*0.4

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}
		}

		TPButton {
			id: btnChooseImage
			imageSource: "choose_avatar.png"
			width: 25
			height: 25

			anchors {
				left: lblChooseImage.right
				leftMargin: 50
				top: parent.top
			}

			onClicked: fileDialog.open();
		}

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}
	}

	Repeater {
		model: 25

		delegate: Rectangle {
			width: windowWidth/5
			height: width
			border.color: "black"
			border.width: 2
			x: (index % 5) * width
			y: Math.floor(index / 5) * width

			Image {
				anchors.fill: parent
				source: "image://tpimageprovider/" + parseInt(index)
			}

			MouseArea {
				anchors.fill: parent
				onClicked: {
					callerWidget.selectAvatar(index);
					avatarsDlg.close();
				}
			}
		}

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			bottom: footerBar.top
		}
	}

	FileDialog {
		id: fileDialog
		title: qsTr("Choose an image to be used as the avatar for the profile")
		nameFilters: [qsTr("Images") + "(*.png *.jpg *.jpeg)"]
		options: FileDialog.ReadOnly
		currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
		fileMode: FileDialog.OpenFile

		onAccepted: {
			callerWidget.selectExternalAvatar(selectedFile);
			avatarsDlg.close();
		}
	}
}
