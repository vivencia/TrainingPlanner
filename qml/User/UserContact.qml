import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

ColumnLayout {
	id: userContactModule

//public:
	required property int userRow
	property bool bReady: bPhoneOK & bEmailOK & bSocialOK

//private:
	property bool bPhoneOK: false
	property bool bEmailOK: false
	property bool bSocialOK: true

	Connections {
		target: AppUserModel
		function onUserModified(row: int, field: int): void {
			if (row === userContactModule.userRow) {
				if (field === 6 || field >= 100)
				userContactModule.getUserInfo();
			}
		}
	}

	onUserRowChanged: getUserInfo();
	Component.onCompleted: getUserInfo();

	TPLabel {
		id: lblPhone
		text: AppUserModel.phoneLabel
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 07
	}

	Row {
		spacing: 10
		Layout.fillWidth: true

		TPPhoneNumberInput {
			id: txtPhoneCountryPrefix
			countryPrefix: true
			readOnly: userContactModule.userRow !== 0
			width: parent.width * 0.2

			onPhoneNumberOKChanged: userContactModule.bPhoneOK = phoneNumberOK;
			onEnterOrReturnKeyPressed: txtPhone.forceActiveFocus();
		}

		TPPhoneNumberInput {
			id: txtPhone
			readOnly: userContactModule.userRow !== 0
			width: parent.width * 0.5

			onPhoneNumberOKChanged: userContactModule.bPhoneOK = phoneNumberOK;
			onEditingFinished: AppUserModel.setPhone(userContactModule.userRow, txtPhoneCountryPrefix.text, text);
			onEnterOrReturnKeyPressed: {
				if (userContactModule.bPhoneOK)
					txtEmail.forceActiveFocus();
			}
		} //txtPhone

		TPButton {
			id: btnWhatsApp
			imageSource: "whatsapp"
			enabled: userContactModule.userRow !== 0 && userContactModule.bPhoneOK
			width: AppSettings.itemDefaultHeight
			height: width

			onClicked: AppOsInterface.startMessagingApp(AppUserModel.phoneNumber(userContactModule.userRow), "WhatsApp");
		}

		TPButton {
			id: btnTelegram
			imageSource: "telegram"
			enabled: userContactModule.userRow !== 0 && userContactModule.bPhoneOK
			width: AppSettings.itemDefaultHeight
			height: width

			onClicked: AppOsInterface.startMessagingApp(AppUserModel.phoneNumber(userContactModule.userRow), "Telegram");
		}
	}

	TPLabel {
		id: lblEmail
		text: AppUserModel.emailLabel
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5
	}

	TPTextInput {
		id: txtEmail
		inputMethodHints: Qt.ImhLowercaseOnly|Qt.ImhEmailCharactersOnly|Qt.ImhNoAutoUppercase
		heightAdjustable: false
		enabled: userContactModule.bPhoneOK
		readOnly: userContactModule.userRow !== 0
		ToolTip.text: AppUserModel.invalidEmailLabel
		Layout.preferredWidth: userContactModule.width - AppSettings.itemDefaultHeight

		onEditingFinished: {
			if (userContactModule.bEmailOK)
				AppUserModel.setEmail(userContactModule.userRow, text);
		}

		onTextEdited: {
			const dot_idx = text.indexOf(".");
			let emailok = false;
			if (dot_idx !== -1) {
				if (dot_idx <= text.length - 3)
					emailok = text.indexOf("@") !== -1;
			}
			ToolTip.visible = !emailok;
			if (AppUserModel.onlineAccount)
				userContactModule.bEmailOK = emailok;
		}

		onEnterOrReturnKeyPressed: {
			if (userContactModule.bEmailOK)
				txtSocial.forceActiveFocus();
		}

		TPButton {
			imageSource: "email"
			enabled: userContactModule.userRow !== 0 && userContactModule.bEmailOK
			width: AppSettings.itemDefaultHeight
			height: width

			onClicked: AppOsInterface.sendMail(txtEmail.text, "", "");

			anchors {
				left: parent.right
				verticalCenter: parent.verticalCenter
			}
		}
	}

	TPLabel {
		id: lblSocial
		text: AppUserModel.socialMediaLabel
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5
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
			txtSocial.text = AppUserModel.socialMedia(userContactModule.userRow, index);
			txtSocial.forceActiveFocus();
		}
	}

	TPTextInput {
		id: txtSocial
		enabled: userContactModule.bEmailOK
		readOnly: userContactModule.userRow !== 0
		heightAdjustable: false
		ToolTip.text: qsTr("Social media address is invalid")
		Layout.preferredWidth: userContactModule.width - AppSettings.itemDefaultHeight

		onEditingFinished: AppUserModel.setSocialMedia(userContactModule.userRow, cboSocial.currentIndex, text);
		onTextEdited: checkSocial()
		onTextChanged: checkSocial();

		function checkSocial(): void {
			userContactModule.bSocialOK = text.length === 0 || text.length > 10;
			ToolTip.visible = !userContactModule.bSocialOK;
		}

		TPButton {
			id: btnOpenSocialMedia
			imageSource: "openurl"
			enabled: userContactModule.bSocialOK
			width: AppSettings.itemDefaultHeight
			height: width

			anchors {
				left: txtSocial.right
				verticalCenter: txtSocial.verticalCenter
			}

			onClicked: AppOsInterface.openURL(txtSocial.text);
		}
	} //txtSocial

	function getUserInfo(): void {
		if (userContactModule.userRow === -1)
			return;
		txtPhoneCountryPrefix.text = AppUserModel.phoneCountryPrefix(userContactModule.userRow);
		txtPhone.text = AppUserModel.phoneNumber(userContactModule.userRow);
		bPhoneOK = AppUserModel.phoneCountryPrefix(userContactModule.userRow).length >= 3 && AppUserModel.phoneNumber(userContactModule.userRow).length === 15
		const email = AppUserModel.email(userContactModule.userRow);
		txtEmail.text = email;
		bEmailOK = !AppUserModel.onlineAccount || email.indexOf("@") !== -1 && email.indexOf(".") !== -1;
		cboSocial.currentIndex = 0;
		txtSocial.text = AppUserModel.socialMedia(userContactModule.userRow, 0);
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
