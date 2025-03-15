import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

Frame {
	id: frmContact
	spacing: 10
	padding: 0
	height: moduleHeight
	implicitHeight: Math.min(height, moduleHeight)

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property int userRow
	property bool bReady: bPhoneOK & bEmailOK & bSocialOK
	property bool bPhoneOK: false
	property bool bEmailOK: false
	property bool bSocialOK: true
	readonly property int nControls: 7
	readonly property int controlsHeight: 25
	readonly property int moduleHeight: nControls*(controlsHeight) + 10

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === userRow && field === 100)
				getUserInfo();
		}
	}

	onUserRowChanged: getUserInfo();
	Component.onCompleted: getUserInfo();

	TPLabel {
		id: lblPhone
		text: userModel.phoneLabel
		height: controlsHeight

		anchors {
			top: parent.top
			topMargin: -5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtPhone
		inputMethodHints: Qt.ImhDigitsOnly
		inputMask: "+55\\(99\\)99999\\-9999;_"
		ToolTip.text: qsTr("Invalid phone number")
		readOnly: userRow !== 0
		height: controlsHeight
		width: frmContact.width*0.7

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

		anchors {
			top: lblPhone.bottom
			left: parent.left
			leftMargin: 5
		}
	}

	TPButton {
		id: btnWhatsApp
		imageSource: "whatsapp"
		enabled: bPhoneOK
		visible: userRow !== 0
		width: 30
		height: 30

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
		width: 30
		height: 30

		anchors {
			left: btnWhatsApp.right
			leftMargin: 5
			verticalCenter: txtPhone.verticalCenter
		}

		onClicked: osInterface.startChatApp(userModel.phone(userRow), "Telegram");
	}

	TPLabel {
		id: lblEmail
		text: userModel.emailLabel
		height: controlsHeight

		anchors {
			top: txtPhone.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtEmail
		inputMethodHints: Qt.ImhLowercaseOnly|Qt.ImhEmailCharactersOnly|Qt.ImhNoAutoUppercase
		enabled: bPhoneOK
		readOnly: userRow !== 0
		ToolTip.text: userModel.invalidEmailLabel
		height: controlsHeight
		width: parent.width*0.9

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

		anchors {
			top: lblEmail.bottom
			left: parent.left
			leftMargin: 5
		}
	}

	TPButton {
		id: btnSendEMail
		imageSource: "email"
		enabled: bEmailOK
		visible: userRow !== 0
		width: 30
		height: 30

		anchors {
			left: txtEmail.right
			leftMargin: 5
			verticalCenter: txtEmail.verticalCenter
		}

		onClicked: osInterface.sendMail(txtEmail.text, "", "");
	}

	TPLabel {
		id: lblSocial
		text: userModel.socialMediaLabel
		height: controlsHeight

		anchors {
			top: txtEmail.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPComboBox {
		id: cboSocial
		height: controlsHeight
		model: socialModel
		completeModel: true
		width: parent.width*0.50

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

		anchors {
			top: lblSocial.bottom
			left: parent.left
			leftMargin: 5
		}
	}

	TPTextInput {
		id: txtSocial
		height: controlsHeight
		enabled: bEmailOK
		readOnly: userRow !== 0
		width: parent.width*0.90
		ToolTip.text: qsTr("Social media address is invalid")

		onEditingFinished: userModel.setSocialMedia(userRow, cboSocial.currentIndex, text);

		onTextEdited: checkSocial()
		onTextChanged: checkSocial();

		anchors {
			top: cboSocial.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
		}

		function checkSocial(): void {
			bSocialOK = text.length === 0 || text.length > 10;
			ToolTip.visible = !bSocialOK;
		}
	}

	TPButton {
		id: btnOpenSocialMedia
		imageSource: "openurl"
		enabled: bSocialOK
		width: 30
		height: 30

		anchors {
			left: txtSocial.right
			verticalCenter: txtSocial.verticalCenter
		}

		onClicked: osInterface.openURL(txtSocial.text);
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
