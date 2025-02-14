import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: passwdDlg
	keepAbove: true
	width: appSettings.pageWidth * 0.8

	required property string title
	required property string message

	property int startYPosition: 0
	property int finalXPos: 0

	onClosed: mainwindow.passwordDialogClosed(1, "");
	onOpened: {
		txtPassword.clear();
		txtPassword.forceActiveFocus();
	}

	TPLabel {
		id: lblTitle
		text: title
		horizontalAlignment: Text.AlignHCenter
		heightAvailable: 50
		width: parent.width - 20

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: btnClose.left
		}
	}

	TPImage {
		id: imgElement
		source: "password"
		width: 50
		height: 50

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: lblMessage.height >= 50 ? lblMessage.verticalCenter : parent.verticalCenter
			top: lblTitle.bottom
			topMargin: 10
		}
	}

	TPLabel {
		id: lblMessage
		text: message
		wrapMode: Text.WordWrap
		heightAvailable: 50
		horizontalAlignment: Text.AlignJustify
		width: passwdDlg.width - imgElement.width - 10
		visible: message.length > 0

		anchors {
			top: lblTitle.bottom
			topMargin: 10
			left: imgElement.right
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtPassword
		echoMode: TextInput.Password
		inputMethodHints: Qt.ImhSensitiveData|Qt.ImhNoPredictiveText
		heightAdjustable: false

		anchors {
			top: lblMessage.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onEnterOrReturnKeyPressed: acceptInput();
	}

	RowLayout {
		spacing: 0
		z: 2

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
			bottomMargin: 5
		}

		TPButton {
			id: btn1
			text: "OK"
			flat: false
			autoResize: true
			enabled: txtPassword.text.length > 4
			Layout.alignment: Qt.AlignCenter

			onClicked: acceptInput();
		}

		TPButton {
			id: btn2
			text: qsTr("Cancel")
			flat: false
			autoResize: true
			Layout.alignment: Qt.AlignCenter
			Layout.maximumWidth: availableWidth - btn1.width - 10

			onClicked: {
				mainwindow.passwordDialogClosed(1, "");
				passwdDlg.closePopup();
			}
		}
	}

	function acceptInput(): void {
		mainwindow.passwordDialogClosed(0, txtPassword.text);
		passwdDlg.closePopup();
	}

	function show(ypos: int): void {
		passwdDlg.height = lblTitle.height + Math.max(imgElement.height, lblMessage.height) + txtPassword.height + btn1.height + 40;
		show1(ypos);
	}
}
