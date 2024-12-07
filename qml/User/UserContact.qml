import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"

Frame {
	id: frmContact
	padding: 0
	spacing: 0
	height: minimumHeight
	implicitHeight: height;
	implicitWidth: width

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property int userRow
	property bool bReady: bPhoneOK & bEmailOK & bSocialOK
	property bool bPhoneOK: false
	property bool bEmailOK: true
	property bool bSocialOK: true
	readonly property int nControls: 7
	readonly property int controlsSpacing: 10
	readonly property int controlsHeight: 30
	readonly property int minimumHeight: nControls*controlsHeight

	TPLabel {
		id: lblPhone
		text: userModel.phoneLabel
		height: controlsHeight

		anchors {
			top: parent.top
			topMargin: (frmContact.availableHeight - minimumHeight)/2
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtPhone
		text: userModel.phone(userRow)
		inputMethodHints: Qt.ImhDigitsOnly
		inputMask: "+55\\(99\\)99999\\-9999\\"
		ToolTip.text: qsTr("Invalid phone number")
		height: controlsHeight
		width: frmContact.width*0.7

		Component.onCompleted: bPhoneOK = userModel.phone(userRow).length >= 17
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
				userModel.setPhone(userRow, text);
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
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtEmail
		text: userModel.email(userRow)
		enabled: bPhoneOK
		ToolTip.text: qsTr("Invalid email address")
		height: controlsHeight
		width: parent.width*0.9

		Component.onCompleted: {
			const str = userModel.email(userRow);
			bEmailOK = (str.length === 0 || (str.indexOf("@") !== -1 && str.indexOf(".") !== -1));
		}

		onEditingFinished: userModel.setEmail(userRow, text);

		onTextEdited: {
			if (text.length === 0 || (text.indexOf("@") !== -1 && text.indexOf(".") !== -1)) {
				ToolTip.visible = false;
				bEmailOK = true;
				if (text.length > 10)
					userModel.setEmail(userRow, text);
			}
			else {
				ToolTip.visible = true;
				bEmailOK = false;
			}
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
			topMargin: controlsSpacing
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
		text: userModel.socialMedia(userRow, cboSocial.currentIndex);
		height: controlsHeight
		enabled: bEmailOK
		width: parent.width*0.90
		ToolTip.text: qsTr("Social media address is invalid")

		onEditingFinished: userModel.setSocialMedia(userRow, cboSocial.currentIndex, text);

		onTextEdited: {
			if (text.length > 10)
				bSocialOK = true;
			else if (text.length === 0)
				bSocialOK === userModel.isEmpty();
			else
				bSocialOK = false;
			ToolTip.visible = !bSocialOK;
		}

		onTextChanged: {
			if (activeFocus) {
				if (text.length > 10) {
					bSocialOK = true;
					userModel.setSocialMedia(userRow, cboSocial.currentIndex, text);
				}
				else if (text.length === 0)
					bSocialOK === userModel.isEmpty();
				else
					bSocialOK = false;
			}
		}

		anchors {
			top: cboSocial.bottom
			topMargin: controlsSpacing
			left: parent.left
			leftMargin: 5
		}
	}

	TPButton {
		id: btnOpenSocialMedia
		imageSource: "openurl"
		enabled: bSocialOK
		visible: userRow !== 0
		width: 30
		height: 30

		anchors {
			left: txtSocial.right
			verticalCenter: txtSocial.verticalCenter
		}

		onClicked: osInterface.openURL(txtSocial.text);
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
