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
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 07
	}

	Row {
		spacing: 10
		Layout.fillWidth: true

		TPPhoneNumberInput {
			id: txtPhoneCountryPrefix
			countryPrefix: true
			readOnly: userRow !== 0
			width: parent.width * 0.2

			onPhoneNumberOKChanged: bPhoneOK = phoneNumberOK;
			onEnterOrReturnKeyPressed: txtPhone.forceActiveFocus();
		}

		TPPhoneNumberInput {
			id: txtPhone
			readOnly: userRow !== 0
			width: parent.width * 0.5

			onPhoneNumberOKChanged: bPhoneOK = phoneNumberOK;
			onEditingFinished: userModel.setPhone(userRow, txtPhoneCountryPrefix.text, text);
			onEnterOrReturnKeyPressed: {
				if (bPhoneOK)
					txtEmail.forceActiveFocus();
			}
		} //txtPhone

		TPButton {
			id: btnWhatsApp
			imageSource: "whatsapp"
			enabled: userRow !== 0 && bPhoneOK
			width: userSettings.itemDefaultHeight
			height: width

			onClicked: osInterface.startChatApp(userModel.phone(userRow), "WhatsApp");
		}

		TPButton {
			id: btnTelegram
			imageSource: "telegram"
			enabled: userRow !== 0 && bPhoneOK
			width: userSettings.itemDefaultHeight
			height: width

			onClicked: osInterface.startChatApp(userModel.phone(userRow), "Telegram");
		}
	}

	TPLabel {
		id: lblEmail
		text: userModel.emailLabel
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5
	}

	TPTextInput {
		id: txtEmail
		inputMethodHints: Qt.ImhLowercaseOnly|Qt.ImhEmailCharactersOnly|Qt.ImhNoAutoUppercase
		heightAdjustable: false
		enabled: bPhoneOK
		readOnly: userRow !== 0
		ToolTip.text: userModel.invalidEmailLabel
		width: userContactModule.width - userSettings.itemDefaultHeight
		Layout.maximumWidth: width
		Layout.minimumWidth: width

		onEditingFinished: {
			if (bEmailOK)
				userModel.setEmail(userRow, text);
		}

		onTextEdited: {
			const dot_idx = text.indexOf(".");
			let emailok = false;
			if (dot_idx !== -1) {
				if (dot_idx <= text.length - 3)
					emailok = text.indexOf("@") !== -1;
			}
			ToolTip.visible = !emailok;
			if (userModel.onlineUser)
				bEmailOK = emailok;
		}

		onEnterOrReturnKeyPressed: {
			if (bEmailOK)
				txtSocial.forceActiveFocus();
		}

		TPButton {
			imageSource: "email"
			enabled: userRow !== 0 && bEmailOK
			width: userSettings.itemDefaultHeight
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
			txtSocial.text = userModel.socialMedia(userRow, index);
			txtSocial.forceActiveFocus();
		}
	}

	TPTextInput {
		id: txtSocial
		enabled: bEmailOK
		readOnly: userRow !== 0
		heightAdjustable: false
		ToolTip.text: qsTr("Social media address is invalid")
		width: userContactModule.width - userSettings.itemDefaultHeight
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
			width: userSettings.itemDefaultHeight
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
		txtPhoneCountryPrefix.text = userModel.phoneCountryPrefix(userRow);
		txtPhone.text = userModel.phoneNumber(userRow);
		bPhoneOK = userModel.phoneCountryPrefix(userRow).length >= 3 && userModel.phoneNumber(userRow).length === 15
		const email = userModel.email(userRow);
		txtEmail.text = email;
		bEmailOK = !userModel.onlineUser || email.indexOf("@") !== -1 && email.indexOf(".") !== -1;
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
