import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import com.vivenciasoftware.qmlcomponents

import "../TPWidgets"
import ".."

TPPopup {
	id: avatarsDlg
	bKeepAbove: false
	width: windowWidth
	height: (repeater.bMale ? 2*windowWidth/5 : 3*windowWidth/5) + 30
	x: 0
	finalYPos: (windowHeight-height)/2;

	required property int userRow
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
			imageSource: "choose_avatar"
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
		id: repeater
		model: bMale ? 10 : 15

		readonly property bool bMale: userModel.sex(userRow) === 0
		readonly property string strSex: bMale ? "m" : "f"

		delegate: Rectangle {
			width: windowWidth/5
			height: width
			border.color: "black"
			border.width: 2
			x: (index % 5) * width
			y: Math.floor(index / 5) * height

			TPImage {
				source: "image://tpimageprovider/" + repeater.strSex + parseInt(index)
				dropShadow: false
				anchors.fill: parent
			}

			MouseArea {
				anchors.fill: parent
				onClicked: {
					callerWidget.selectAvatar(repeater.strSex + parseInt(index));
					avatarsDlg.close();
				}
			}
		}

		anchors {
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
			callerWidget.selectExternalAvatar(appUtils.getCorrectPath(currentFile));
			avatarsDlg.close();
		}
	}
}
