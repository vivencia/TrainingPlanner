import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

ColumnLayout {
	id: userModule
	spacing: 5

	required property int userRow
	property bool bReady: bPhoneOK & bEmailOK & bSocialOK
	property bool bPhoneOK: false
	property bool bEmailOK: false
	property bool bSocialOK: true

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === userRow && field >= 100)
				getUserInfo();
		}
	}

	onUserRowChanged: getUserInfo();
	Component.onCompleted: getUserInfo();

	TPLabel {
		id: lblPhone
		text: userModel.phoneLabel
	}

	TPTextInput {
		id: txtPhone
		inputMethodHints: Qt.ImhDigitsOnly
		inputMask: "+55\\(99\\)99999\\-9999;_"
		ToolTip.text: qsTr("Invalid phone number")
		readOnly: userRow !== 0
		Layout.maximumWidth: userModule.width*0.85
		Layout.minimumWidth: userModule.width*0.85

		onEditingFinished: userModel.setPhone(userRow, text);

		onActiveFocusChanged: {
			if (activeFocus) {
				if (text.length < 17)
					cursorPosition = 4;
				else
					cursorPosition = 17;
			}
		}

		onEnterOrReturnKeyPressed: {
			if (bPhoneOK)
				txtEmail.forceActiveFocus();
		}

		onTextEdited: {
			if (text.length === 17) {
				ToolTip.visible = false;
				bPhoneOK = true;
			}
			else {
				ToolTip.visible = true;
				bPhoneOK = false;
			}
		}

		TPButton {
			id: btnWhatsApp
			imageSource: "whatsapp"
			enabled: bPhoneOK
			visible: userRow !== 0
			width: appSettings.itemDefaultHeight
			height: width

			anchors {
				left: txtPhone.right
				leftMargin: 5
				verticalCenter: txtPhone.verticalCenter
			}

			onClicked: osInterface.startChatApp(userModel.phone(userRow), "WhatsApp");
		}

		TPButton {
			id: btnTelegram
			imageSource: "telegram"
			enabled: bPhoneOK
			visible: userRow !== 0
			width: appSettings.itemDefaultHeight
			height: width

			anchors {
				left: btnWhatsApp.right
				leftMargin: 5
				verticalCenter: txtPhone.verticalCenter
			}

			onClicked: osInterface.startChatApp(userModel.phone(userRow), "Telegram");
		}
	} //txtPhone

	TPLabel {
		id: lblEmail
		text: userModel.emailLabel
	}

	TPTextInput {
		id: txtEmail
		inputMethodHints: Qt.ImhLowercaseOnly|Qt.ImhEmailCharactersOnly|Qt.ImhNoAutoUppercase
		enabled: bPhoneOK
		readOnly: userRow !== 0
		ToolTip.text: userModel.invalidEmailLabel
		Layout.maximumWidth: userModule.width*0.85
		Layout.minimumWidth: userModule.width*0.85

		onEditingFinished: userModel.setEmail(userRow, text);

		onTextEdited: {
			const dot_idx = text.indexOf(".");
			let emailok = false;
			if (dot_idx !== -1) {
				if (dot_idx <= text.length - 3)
					emailok = text.indexOf("@") !== -1;
			}
			ToolTip.visible = !emailok;
			bEmailOK = emailok;
		}

		onEnterOrReturnKeyPressed: {
			if (bEmailOK)
				txtSocial.forceActiveFocus();
		}

		TPButton {
			imageSource: "email"
			enabled: bEmailOK
			visible: userRow !== 0
			width: appSettings.itemDefaultHeight
			height: width

			onClicked: osInterface.sendMail(txtEmail.text, "", "");
		}
	}

	TPLabel {
		id: lblSocial
		text: userModel.socialMediaLabel
	}

	TPComboBox {
		id: cboSocial
		model: socialModel
		completeModel: true
		Layout.preferredWidth: userModule.width*0.6

		ListModel {
			id: socialModel
			ListElement { text: "YouTube"; icon: "youtube"; value: 0; enabled: true; }
			ListElement { text: "Twitter"; icon: "twitter"; value: 1; enabled: true; }
			ListElement { text: "Instagram"; icon: "instagram"; value: 2; enabled: true; }
			ListElement { text: "Facebook"; icon: "facebook"; value: 3; enabled: true; }
			ListElement { text: qsTr("Other"); icon: "www"; value: 4; enabled: true; }
		}

		onActivated: (index) => {
			txtSocial.text = userModel.socialMedia(userRow, index);
			txtSocial.forceActiveFocus();
		}
	}

	TPTextInput {
		id: txtSocial
		enabled: bEmailOK
		readOnly: userRow !== 0
		Layout.maximumWidth: userModule.width*0.6
		Layout.minimumWidth: userModule.width*0.6
		ToolTip.text: qsTr("Social media address is invalid")

		onEditingFinished: userModel.setSocialMedia(userRow, cboSocial.currentIndex, text);
		onTextEdited: checkSocial()
		onTextChanged: checkSocial();

		function checkSocial(): void {
			bSocialOK = text.length === 0 || text.length > 10;
			ToolTip.visible = !bSocialOK;
		}

		TPButton {
			id: btnOpenSocialMedia
			imageSource: "openurl"
			enabled: bSocialOK
			width: appSettings.itemDefaultHeight
			height: width

			anchors {
				left: txtSocial.right
				verticalCenter: txtSocial.verticalCenter
			}

			onClicked: osInterface.openURL(txtSocial.text);
		}
	} //txtSocial

	function getUserInfo(): void {
		if (userRow === -1)
			return;
		txtPhone.text = userModel.phone(userRow);
		bPhoneOK = txtPhone.text.length >= 17
		const email = userModel.email(userRow);
		txtEmail.text = email;
		bEmailOK = email.indexOf("@") !== -1 && email.indexOf(".") !== -1;
		cboSocial.currentIndex = 0;
		txtSocial.text = userModel.socialMedia(userRow, 0);
	}

	function focusOnFirstField() {
		if (!bPhoneOK)
			txtPhone.forceActiveFocus();
		else if (!bEmailOK)
			txtEmail.forceActiveFocus();
		else
			txtSocial.forceActiveFocus();
	}
}
