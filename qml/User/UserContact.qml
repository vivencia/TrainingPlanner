import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

ColumnLayout {
	id: userContactModule

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
		Layout.fillWidth: true
		Layout.topMargin: 10
	}

	TPPhoneNumberInput {
		id: txtPhone
		readOnly: userRow !== 0
		width: userContactModule.width - 2*appSettings.itemDefaultHeight
		Layout.maximumWidth: width
		Layout.minimumWidth: width

		onPhoneNumberOKChanged: bPhoneOK = phoneNumberOK;
		onEditingFinished: userModel.setPhone(userRow, text);

		onEnterOrReturnKeyPressed: {
			if (bPhoneOK)
				txtEmail.forceActiveFocus();
		}

		TPButton {
			id: btnWhatsApp
			imageSource: "whatsapp"
			enabled: userRow !== 0 && bPhoneOK
			width: appSettings.itemDefaultHeight
			height: width

			anchors {
				left: txtPhone.right
				verticalCenter: txtPhone.verticalCenter
			}

			onClicked: osInterface.startChatApp(userModel.phone(userRow), "WhatsApp");
		}

		TPButton {
			id: btnTelegram
			imageSource: "telegram"
			enabled: userRow !== 0 && bPhoneOK
			width: appSettings.itemDefaultHeight
			height: width

			anchors {
				left: btnWhatsApp.right
				verticalCenter: txtPhone.verticalCenter
			}

			onClicked: osInterface.startChatApp(userModel.phone(userRow), "Telegram");
		}
	} //txtPhone

	TPLabel {
		id: lblEmail
		text: userModel.emailLabel
		Layout.fillWidth: true
		Layout.topMargin: 10
	}

	TPTextInput {
		id: txtEmail
		inputMethodHints: Qt.ImhLowercaseOnly|Qt.ImhEmailCharactersOnly|Qt.ImhNoAutoUppercase
		enabled: bPhoneOK
		readOnly: userRow !== 0
		ToolTip.text: userModel.invalidEmailLabel
		width: userContactModule.width - appSettings.itemDefaultHeight
		Layout.maximumWidth: width
		Layout.minimumWidth: width

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
			enabled: userRow !== 0 && bEmailOK
			width: appSettings.itemDefaultHeight
			height: width

			onClicked: osInterface.sendMail(txtEmail.text, "", "");

			anchors {
				left: parent.right
				verticalCenter: parent.verticalCenter
			}
		}
	}

	TPLabel {
		id: lblSocial
		text: userModel.socialMediaLabel
		Layout.fillWidth: true
		Layout.topMargin: 10
	}

	TPComboBox {
		id: cboSocial
		model: socialModel
		completeModel: true
		Layout.minimumWidth: userContactModule.width*0.6

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
		ToolTip.text: qsTr("Social media address is invalid")
		width: userContactModule.width - appSettings.itemDefaultHeight
		Layout.maximumWidth: width
		Layout.minimumWidth: width

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
