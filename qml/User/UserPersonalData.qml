import QtQuick
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"

Frame {
	height: 175

	property bool bReady: readyBlocks[0] && readyBlocks[1] && readyBlocks[2]
	property var readyBlocks: [false,false,false]

	Label {
		id: lblName
		text: userModel.columnLabel(1)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: 25

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtName
		text: userModel.userName
		height: 25
		ToolTip.text: qsTr("The name is too short")

		onTextChanged: userModel.userName = text;

		onEnterOrReturnKeyPressed: {
			if (txtBirthdate.enabled)
				txtBirthdate.forceActiveFocus();
		}

		onTextEdited: {
			if (text.length >=5) {
				txtBirthdate.enabled = true;
				ToolTip.visible = false;
				readyBlocks[0] = true;
			}
			else {
				txtBirthdate.enabled = false;
				ToolTip.visible = true;
				readyBlocks[0] = false;
			}
		}

		anchors {
			top: lblName.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	Label {
		id: lblBirthdate
		text: userModel.columnLabel(2)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: 25

		anchors {
			top: txtName.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtBirthdate
		text: userModel.userBirthday
		readOnly: true
		enabled: userModel.count > 0
		height: 25

		onTextEdited: {
			frmSex.enabled = acceptableInput;
			ToolTip.visible = !acceptableInput;
		}

		anchors {
			top: lblBirthdate.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		CalendarDialog {
			id: caldlg
			showDate: new Date()
			initDate: new Date(1940, 0, 1)
			finalDate: new Date()
			parentPage: homePage

			onDateSelected: (date) => {
				txtBirthdate = runCmd.formatDate(date);
				readyBlocks[1] = true;
				frmSex.enabled = true;
			}
		}

		TPRoundButton {
			id: btnStartDate
			width: 25
			height: 25
			imageName: "calendar.png"

			onClicked: caldlg.open();

			anchors.left: txtBirthdate.right
			anchors.verticalCenter: txtBirthdate.verticalCenter
		}
	}

	Frame {
		id: frmSex
		enabled: userModel.count > 0
		height: 25

		TPRadioButton {
			text: qsTr("Male")
			height: 25
			checked: userModel.userSex === qsTr("Male")

			onClicked: readyBlocks[2] = true;

			anchors {
				verticalCenter: parent.verticalCenter
				left: parent.left
				leftMargin: 10
			}
		}

		TPRadioButton {
			text: qsTr("Female")
			height: 25
			checked: userModel.userSex === qsTr("Female")

			onClicked: readyBlocks[2] = true;

			anchors {
				verticalCenter: parent.verticalCenter
				right: parent.right
				rightMargin: 10
			}
		}

		anchors {
			top: txtBirthdate.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}
}
