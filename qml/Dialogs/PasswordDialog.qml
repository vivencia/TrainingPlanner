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
	height: mainLayout.childrenRect.height * 1.1

	required property string title
	required property string message

	property int startYPosition: 0
	property int finalXPos: 0

	onClosed: mainwindow.passwordDialogClosed(1, "");
	onOpened: {
		txtPassword.clear();
		txtPassword.forceActiveFocus();
	}

	ColumnLayout {
		id: mainLayout
		spacing: 10
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			margins: 5
		}

		TPLabel {
			id: lblTitle
			text: title
			horizontalAlignment: Text.AlignHCenter
			visible: title.length > 0
			width: parent.width - 20
		}

		RowLayout {
			Layout.fillWidth: true

			TPImage {
				id: imgElement
				source: "password"
				Layout.preferredWidth: appSettings.itemExtraLargeHeight
				Layout.preferredHeight: appSettings.itemExtraLargeHeight
				Layout.alignment: Qt.AlignVCenter
			}

			TPLabel {
				id: lblMessage
				text: message
				singleLine: false
				horizontalAlignment: Text.AlignJustify
				width: passwdDlg.width - imgElement.width - 10
				visible: message.length > 0
				Layout.fillWidth: true
			}
		}

		TPPasswordInput {
			id: txtPassword
			Layout.fillWidth: true

			onEnterOrReturnKeyPressed: acceptInput();
		}

		RowLayout {
			id: buttonsRow
			spacing: (passwdDlg.width - btn1.width - btn2.width) / 2
			Layout.alignment: Qt.AlignHCenter

			TPButton {
				id: btn1
				text: "OK"
				autoSize: true
				enabled: txtPassword.text.length > 4
				Layout.alignment: Qt.AlignHCenter

				onClicked: acceptInput();
			}

			TPButton {
				id: btn2
				text: qsTr("Cancel")
				autoSize: true
				Layout.alignment: Qt.AlignHCenter

				onClicked: {
					mainwindow.passwordDialogClosed(1, "");
					passwdDlg.closePopup();
				}
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
