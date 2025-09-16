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
	height: appSettings.pageHeight * 0.4

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
		width: parent.width - 20
		visible: title.length > 0

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
		width: userSettings.itemDefaultHeight * 2
		height: width

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: lblMessage.lineCount > 1 ? lblMessage.verticalCenter : parent.verticalCenter
			top: lblTitle.bottom
			topMargin: 10
		}
	}

	TPLabel {
		id: lblMessage
		text: message
		singleLine: false
		horizontalAlignment: Text.AlignJustify
		width: passwdDlg.width - imgElement.width - 10
		visible: message.length > 0

		anchors {
			top: lblTitle.bottom
			left: imgElement.right
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: txtPassword.top
		}
	}

	TPPasswordInput {
		id: txtPassword

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: buttonsRow.top
			bottomMargin: 10
		}

		onEnterOrReturnKeyPressed: acceptInput();
	}

	RowLayout {
		id: buttonsRow
		spacing: 0

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
			autoSize: true
			enabled: txtPassword.text.length > 4
			Layout.alignment: Qt.AlignCenter

			onClicked: acceptInput();
		}

		TPButton {
			id: btn2
			text: qsTr("Cancel")
			autoSize: true
			Layout.alignment: Qt.AlignCenter

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
		show1(ypos);
	}
}
