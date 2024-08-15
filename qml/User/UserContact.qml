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

	required property int userRow
	property bool bReady: bPhoneOK & bEmailOK & bSocialOK
	property bool bPhoneOK: false
	property bool bEmailOK: true
	property bool bSocialOK: true
	readonly property int controlsSpacing: 5
	readonly property int controlsHeight: 25

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
		height: controlsHeight
		text: userModel.phone(userRow)
		inputMethodHints: Qt.ImhDigitsOnly
		inputMask: "+55 \\(99\\) 99999\\-9999;_"
		ToolTip.text: qsTr("Invalid phone number")

		Component.onCompleted: bPhoneOK = userModel.phone(userRow).length >= 19
		onTextChanged: userModel.setPhone(userRow, text);

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
		height: controlsHeight
		enabled: bPhoneOK
		ToolTip.text: qsTr("Invalid email address")

		Component.onCompleted: {
			const str = userModel.email(userRow);
			bEmailOK = (str.length === 0 || (str.indexOf("@") !== -1 && str.indexOf(".") !== -1));
		}

		onTextChanged: userModel.setEmail(userRow, text);

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
			txtSocial.text = runCmd.getCompositeValue(index, userModel.socialMedia(userRow));
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
		text: runCmd.getCompositeValue(cboSocial.currentIndex, userModel.socialMedia(userRow));
		height: controlsHeight
		enabled: bEmailOK
		width: parent.width*0.65
		ToolTip.text: qsTr("Social media address is invalid")

		onTextChanged: userModel.setSocialMedia(userRow, runCmd.setCompositeValue_QML(cboSocial.currentIndex, text, userModel.socialMedia(userRow)));

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
		if (!bPhoneOK)
			txtPhone.forceActiveFocus();
		else if (!bEmailOK)
			txtEmail.forceActiveFocus();
		else
			txtSocial.forceActiveFocus();
	}
}
