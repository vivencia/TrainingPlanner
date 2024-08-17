import QtQuick
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

import ".."
import "../TPWidgets"

Frame {
	id: frmContact
	implicitHeight: moduleHeight;
	implicitWidth: width
	padding: 0
	spacing: 0

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property int userRow
	property bool bReady: bPhoneOK & bEmailOK & bSocialOK
	property bool bPhoneOK: false
	property bool bEmailOK: true
	property bool bSocialOK: true
	readonly property int controlsSpacing: 5
	readonly property int controlsHeight: 25
	readonly property int moduleHeight: 7*controlsHeight

	Label {
		id: lblPhone
		text: userModel.columnLabel(4)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		bottomPadding: 0

		anchors {
			top: parent.top
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

		Component.onCompleted: bPhoneOK = userModel.phone(userRow).length >= 19
		onEditingFinished: userModel.setPhone(userRow, text);

		onActiveFocusChanged: {
			if (activeFocus) {
				if (text.length < 19)
					cursorPosition = 5;
				else
					cursorPosition = 19;
			}
		}

		onEnterOrReturnKeyPressed: {
			if (bPhoneOK)
				txtEmail.forceActiveFocus();
		}

		onTextEdited: {
			if (text.length === 19) {
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
		imageSource: "qrc:/images/whatsapp.png"
		enabled: bPhoneOK
		visible: userRow !== 0
		width: 30
		height: 30

		anchors {
			left: txtPhone.right
			verticalCenter: txtPhone.verticalCenter
		}

		onClicked: appDB.startChatApp(userModel.phone(userRow), "WhatsApp");
	}

	TPButton {
		id: btnTelegram
		imageSource: "qrc:/images/telegram.png"
		enabled: bPhoneOK
		visible: userRow !== 0
		width: 30
		height: 30

		anchors {
			left: btnWhatsApp.right
			verticalCenter: txtPhone.verticalCenter
		}

		onClicked: appDB.startChatApp(userModel.phone(userRow), "Telegram");
	}

	Label {
		id: lblEmail
		text: userModel.columnLabel(5)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		topPadding: 0
		bottomPadding: 0

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
		imageSource: "qrc:/images/email.png"
		enabled: bEmailOK
		visible: userRow !== 0
		width: 30
		height: 30

		anchors {
			left: txtEmail.right
			verticalCenter: txtEmail.verticalCenter
		}

		onClicked: appDB.sendMail(txtEmail.text, "", "");
	}

	Label {
		id: lblSocial
		text: userModel.columnLabel(6)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0

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
			ListElement { text: qsTr("YouTube"); icon: "qrc:/images/youtube.png"; value: 0; }
			ListElement { text: qsTr("Twitter"); icon: "qrc:/images/twitter.png"; value: 1; }
			ListElement { text: qsTr("Instagram"); icon: "qrc:/images/instagram.png"; value: 2; }
			ListElement { text: qsTr("Facebook"); icon: "qrc:/images/facebook.png"; value: 3; }
			ListElement { text: qsTr("Other"); icon: "qrc:/images/www.png"; value: 4; }
		}

		onActivated: (index) => {
			txtSocial.text = runCmd.getCompositeValue(index, userModel.socialMedia(userRow));
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
		text: runCmd.getCompositeValue(cboSocial.currentIndex, userModel.socialMedia(userRow));
		height: controlsHeight
		enabled: bEmailOK
		width: parent.width*0.90
		ToolTip.text: qsTr("Social media address is invalid")

		onEditingFinished: userModel.setSocialMedia(userRow, runCmd.setCompositeValue_QML(cboSocial.currentIndex, text, userModel.socialMedia(userRow)));

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
					userModel.setSocialMedia(userRow, runCmd.setCompositeValue_QML(cboSocial.currentIndex, text, userModel.socialMedia(userRow)));
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
		imageSource: "openurl.png"
		enabled: bSocialOK
		visible: userRow !== 0
		width: 30
		height: 30

		anchors {
			left: txtSocial.right
			verticalCenter: txtSocial.verticalCenter
		}

		onClicked: appDB.openURL(txtSocial.text);
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
