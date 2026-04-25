pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Widgets
import TpQml.Dialogs

TPPopup {
	id: avatarsDlg
	closeButtonVisible: false
	showTitleBar: false
	open_in_window: true
	width: AppSettings.pageWidth
	height: (repeater.bMale ? 2 * AppSettings.pageWidth / 5 : 3 * AppSettings.pageWidth/5) + 30

//public:
	required property int userRow

	signal avatarSelected(string id, bool from_file);

	Rectangle {
		id: footerBar
		width: AppSettings.pageWidth
		height: AppSettings.itemDefaultHeight
		color: "transparent"

		TPLabel {
			id: lblChooseImage
			text: qsTr("Choose another image...")
			topPadding: 5
			leftPadding: 10
			width: parent.width * 0.4

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}
		}

		TPButton {
			id: btnChooseImage
			imageSource: "choose-avatar"
			width: AppSettings.itemDefaultHeight
			height: AppSettings.itemDefaultHeight

			anchors {
				left: lblChooseImage.right
				leftMargin: 50
				top: parent.top
			}

			onClicked: fileDialogLoader.active = true;
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

		readonly property bool bMale: AppUserModel.sex(avatarsDlg.userRow) === 0
		readonly property string strSex: bMale ? "m" : "f"

		delegate: Rectangle {
			id: delegate
			width: AppSettings.pageWidth / 5
			height: width
			border.color: "black"
			border.width: 2
			x: (index % 5) * width
			y: Math.floor(index / 5) * height

			required property int index

			TPImage {
				source: "image://tpimageprovider/" + repeater.strSex + parseInt(delegate.index)
				dropShadow: false
				anchors.fill: parent
			}

			MouseArea {
				anchors.fill: parent
				onClicked: {
					avatarsDlg.avatarSelected(repeater.strSex + parseInt(delegate.index), false);
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

	Loader {
		id: fileDialogLoader
		asynchronous: true
		active: false

		property TPFileDialog _file_dialog
		sourceComponent: TPFileDialog {
			id: fileDialog
			title: qsTr("Choose an image to be used as the avatar for the profile")
			includeImageFilter: true

			onAccepted: {
				avatarsDlg.avatarSelected(AppUtils.getCorrectPath(currentFile), true);
				fileDialogLoader.active = false;
				avatarsDlg.close();
			}
			onRejected: fileDialogLoader.active = false;
			Component.onCompleted: fileDialogLoader._file_dialog = this;
		}

		onLoaded: _file_dialog.show();
	}
}
