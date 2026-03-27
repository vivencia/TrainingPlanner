import QtQuick
import QtQuick.Layouts


import TpQml
import TpQml.Widgets

TPPopup {
	id: _passwdDlg
	keepAbove: true
	width: AppSettings.pageWidth * 0.8
	height: mainLayout.childrenRect.height * 1.1

	required property string title
	required property string message

	onOpened: {
		txtPassword.clear();
		txtPassword.forceActiveFocus();
	}

	ColumnLayout {
		id: mainLayout
		spacing: 10

		anchors {
			top: _passwdDlg.contentItem.top
			left: _passwdDlg.contentItem.left
			right: _passwdDlg.contentItem.right
			margins: 5
		}

		TPLabel {
			id: lblTitle
			text: _passwdDlg.title
			horizontalAlignment: Text.AlignHCenter
			visible: _passwdDlg.title.length > 0
			Layout.preferredWidth: parent.width - 20
		}

		RowLayout {
			Layout.fillWidth: true

			TPImage {
				id: imgElement
				source: "password"
				Layout.preferredWidth: AppSettings.itemExtraLargeHeight
				Layout.preferredHeight: AppSettings.itemExtraLargeHeight
				Layout.alignment: Qt.AlignVCenter
			}

			TPLabel {
				id: lblMessage
				text: _passwdDlg.message
				singleLine: false
				horizontalAlignment: Text.AlignJustify
				visible: _passwdDlg.message.length > 0
				Layout.preferredWidth: _passwdDlg.width - imgElement.width - 10
				Layout.fillWidth: true
			}
		}

		TPPasswordInput {
			id: txtPassword
			Layout.fillWidth: true

			onEnterOrReturnKeyPressed: _passwdDlg.acceptInput();
		}

		RowLayout {
			id: buttonsRow
			spacing: (_passwdDlg.width - btn1.width - btn2.width) / 2
			Layout.alignment: Qt.AlignHCenter

			TPButton {
				id: btn1
				text: "OK"
				autoSize: true
				enabled: txtPassword.text.length > 4
				Layout.alignment: Qt.AlignHCenter

				onClicked: _passwdDlg.acceptInput();
			}

			TPButton {
				id: btn2
				text: qsTr("Cancel")
				autoSize: true
				Layout.alignment: Qt.AlignHCenter

				onClicked: _passwdDlg.closePopup();
			}
		}
	}

	function acceptInput(): void {
		AppUserModel.checkPassword(txtPassword.text);
		_passwdDlg.closePopup();
	}
}
