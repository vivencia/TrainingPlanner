import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

ColumnLayout {
	id: userContactModule
	spacing: 10

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

	TPTextInput {
		id: txtPhone
		inputMethodHints: Qt.ImhDigitsOnly
		validator: RegularExpressionValidator { regularExpression: /\+?[0-9]{0,2} ?(\([0-9]{0,2}\))? ?[0-9]{0,5}-?[0-9]{0,4}/ }
		placeholderText: "+55 (XX) XXXXX-XXXX"
		maximumLength: 19
		ToolTip.text: qsTr("Invalid phone number")
		readOnly: userRow !== 0
		Layout.maximumWidth: userContactModule.width * 0.85
		Layout.minimumWidth: userContactModule.width * 0.85

		onEditingFinished: userModel.setPhone(userRow, text);

		onActiveFocusChanged: {
			if (activeFocus) {
				if (text.length < 19)
					cursorPosition = 4;
				else
					cursorPosition = 19;
			}
		}

		onEnterOrReturnKeyPressed: {
			if (bPhoneOK)
				txtEmail.forceActiveFocus();
		}

		onTextEdited: {
			let oldText = text;
			let oldCursor = cursorPosition;
			let digits = oldText.replace(/\D/g, '');
			text = formatPhoneNumber(digits);
			cursorPosition = adjustCursorPosition(oldText, text, oldCursor);
			if (text.length === 19) {
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
	}

	TPTextInput {
		id: txtEmail
		inputMethodHints: Qt.ImhLowercaseOnly|Qt.ImhEmailCharactersOnly|Qt.ImhNoAutoUppercase
		enabled: bPhoneOK
		readOnly: userRow !== 0
		ToolTip.text: userModel.invalidEmailLabel
		Layout.maximumWidth: userContactModule.width*0.85
		Layout.minimumWidth: userContactModule.width*0.85

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
		Layout.maximumWidth: userContactModule.width*0.85
		Layout.minimumWidth: userContactModule.width*0.85

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

	function formatPhoneNumber(digits: string) : string {
		// Remove all non-digits
		digits = digits.replace(/\D/g, '');

		// Format based on length
		let formatted = "";
		if (digits.length === 0)
			return ""; // Empty input shows nothing
		else if (digits.length <= 2)
			formatted = "+" + digits;
		else if (digits.length <= 4)
			formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2) + ")";
		else if (digits.length <= 9)
			formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2, 4) + ") " + digits.substring(4);
		else
			formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2, 4) + ") " + digits.substring(4, 9) + "-" + digits.substring(9);
		return formatted;
	}

	// Function to calculate cursor position after formatting
	function adjustCursorPosition(oldText: string, newText: string, oldCursor: int) : void {
		// Count non-digit characters (formatting chars) up to old cursor position
		let nonDigitsBeforeCursor = oldText.substring(0, oldCursor).replace(/[0-9]/g, '').length;
		// Count digits up to old cursor position
		let digitsBeforeCursor = oldText.substring(0, oldCursor).replace(/\D/g, '').length;
		// Calculate new cursor position in formatted text
		let newFormatted = formatPhoneNumber(newText.replace(/\D/g, ''));
		let digitCount = 0;
		let nonDigitCount = 0;
		for (let i = 0; i < newFormatted.length; i++) {
			if (/\d/.test(newFormatted[i])) {
				digitCount++;
				if (digitCount === digitsBeforeCursor) {
					return i + 1; // Place cursor after the current digit
				}
			} else {
				nonDigitCount++;
			}
		}
		return newFormatted.length; // Fallback to end of text
	}

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
