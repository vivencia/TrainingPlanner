import QtQuick
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

import ".."
import "../TPWidgets"

Frame {
	implicitHeight: height
	implicitWidth: width
	padding: 0
	spacing: 0

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	property bool bReady: bPhoneOK & bEmailOK & bSocialOK
	property bool bPhoneOK: false
	property bool bEmailOK: true
	property bool bSocialOK: true
	readonly property int controlsSpacing: 10
	readonly property int controlsHeight: height / 6

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
		height: controlsHeight
		inputMethodHints: Qt.ImhDigitsOnly
		inputMask: "+55 \\ (99\\) 99999\\-9999;_"
		ToolTip.text: qsTr("Invalid phone number")

		Component.onCompleted: {
			text = userModel.phone;
			bPhoneOK = !userModel.isEmpty();
		}

		onActiveFocusChanged: {
			if (activeFocus) {
				if (text.length < 20)
					cursorPosition = 5;
				else
					cursorPosition = 20;
			}
		}

		onTextChanged: userModel.phone = text;

		onEnterOrReturnKeyPressed: {
			if (bPhoneOK)
				txtEmail.forceActiveFocus();
		}

		onTextEdited: {
			if (text.length === 20) {
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
			topMargin: -10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	Label {
		id: lblEmail
		text: userModel.columnLabel(5)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0

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
		height: controlsHeight
		enabled: bPhoneOK
		ToolTip.text: qsTr("Invalid email address")

		Component.onCompleted: text = userModel.email;

		onTextChanged: userModel.email = text;

		onTextEdited: {
			if (text.length === 0 || (text.indexOf("@") !== -1 && text.indexOf(".") !== -1)) {
				ToolTip.visible = false;
				bEmailOK = true;
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
			topMargin: -10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
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
		width: parent.width*0.35

		ListModel {
			id: socialModel
			ListElement { text: qsTr("YouTube"); icon: "qrc:/images/youtube.png"; value: 0; }
			ListElement { text: qsTr("Twitter"); icon: "qrc:/images/twitter.png"; value: 1; }
			ListElement { text: qsTr("Instagram"); icon: "qrc:/images/instagram.png"; value: 2; }
			ListElement { text: qsTr("Facebook"); icon: "qrc:/images/facebook.png"; value: 3; }
			ListElement { text: qsTr("Other"); icon: "qrc:/images/www.png"; value: 4; }
		}

		onActivated: (index) => {
			txtSocial.text = runCmd.getCompositeValue(index, userModel.socialMedia);
			txtSocial.forceActiveFocus();
		}

		anchors {
			top: lblSocial.bottom
			topMargin: -10
			left: parent.left
			leftMargin: 5
		}
	}

	TPTextInput {
		id: txtSocial
		height: controlsHeight
		enabled: bEmailOK
		width: parent.width*0.65
		ToolTip.text: qsTr("Social media address is invalid")

		Component.onCompleted: text = runCmd.getCompositeValue(cboSocial.currentIndex, userModel.socialMedia);
		onTextChanged: userModel.socialMedia = runCmd.setCompositeValue_QML(cboSocial.currentIndex, text, userModel.socialMedia);

		onTextEdited: {
			bSocialOK = text.length > 10 || text.length === 0
			ToolTip.visible = !bSocialOK;
		}

		anchors {
			top: lblSocial.bottom
			topMargin: -10
			left: cboSocial.right
			leftMargin: 0
			right: parent.right
			rightMargin: 5
		}
	}

	function focusOnFirstField() {
		if (txtPhone.text.length < 20)
			txtPhone.forceActiveFocus();
	}
}
