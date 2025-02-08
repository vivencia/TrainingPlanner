import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"
import "../Pages"

Frame {
	id: frmUserData
	spacing: 15
	padding: 0
	height: moduleHeight
	implicitHeight: Math.min(height, moduleHeight)

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property TPPage parentPage
	required property int userRow
	property bool bReady: bNameOK && bBirthDateOK && bSexOK
	property bool bNameOK
	property bool bBirthDateOK
	property bool bSexOK
	readonly property int nControls: 5
	readonly property int controlsHeight: 25
	readonly property int moduleHeight: nControls*(controlsHeight) + 15

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === userRow && field === 100)
				getUserInfo();
		}
	}

	TPLabel {
		id: lblName
		text: userModel.nameLabel
		height: controlsHeight

		anchors {
			top: parent.top
			topMargin: (availableHeight - moduleHeight)/2
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtName
		height: controlsHeight
		ToolTip.text: qsTr("The name is too short")

		property bool bTextChanged: false

		onEditingFinished: {
			if (bTextChanged && bNameOK) {
				userModel.setUserName(userRow, text);
				bTextChanged = false;
			}
		}

		onTextEdited: {
			bTextChanged = true;
			if (text.length >= 5) {
				ToolTip.visible = false;
				bNameOK = true;
			}
			else {
				ToolTip.visible = true;
				bNameOK = false;
			}
		}

		anchors {
			top: lblName.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPLabel {
		id: lblBirthdate
		text: userModel.birthdayLabel
		height: controlsHeight

		anchors {
			top: txtName.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtBirthdate
		readOnly: true
		height: controlsHeight
		enabled: bNameOK

		anchors {
			top: lblBirthdate.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5 + btnBirthDate.width
		}

		CalendarDialog {
			id: caldlg
			showDate: userModel.birthDate(userRow)
			initDate: new Date(1940, 0, 1)
			finalDate: new Date()
			parentPage: frmUserData.parentPage

			onDateSelected: (date) => {
				userModel.setBirthDate(userRow, date);
				txtBirthdate.text = userModel.birthDateFancy(userRow);
				bBirthDateOK = true;
				if (txtName.text.length === 0)
					txtName.forceActiveFocus();
			}
		}

		TPButton {
			id: btnBirthDate
			imageSource: "calendar.png"
			imageSize: 30
			anchors.left: txtBirthdate.right
			anchors.verticalCenter: txtBirthdate.verticalCenter

			onClicked: caldlg.open();
		}
	}

	Pane {
		id: frmSex
		enabled: bBirthDateOK
		height: controlsHeight
		padding: 0
		spacing: 0

		background: Rectangle {
			color: "transparent"
		}

		anchors {
			top: txtName.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		TPRadioButton {
			id: chkMale
			text: qsTr("Male")
			height: controlsHeight
			width: frmSex.width/2

			onClicked: {
				bSexOK = true;
				chkFemale.checked = !checked;
				if (checked) {
					userModel.setSex(userRow, 0);
					userModel.setAvatar(userRow, "image://tpimageprovider/m5");
				}
			}

			anchors {
				verticalCenter: parent.verticalCenter
				left: parent.left
				leftMargin: 10
			}
		}

		TPRadioButton {
			id: chkFemale
			text: qsTr("Female")
			height: controlsHeight
			width: frmSex.width/2

			onClicked: {
				bSexOK = true;
				chkMale.checked = !checked;
				if (checked) {
					userModel.setSex(userRow, 1);
					userModel.setAvatar(userRow, "image://tpimageprovider/f0");
				}
			}

			anchors {
				verticalCenter: parent.verticalCenter
				right: parent.right
				rightMargin: 10
			}
		}
	}

	function getUserInfo(): void {
		txtName.text = userModel.userName(userRow);
		bNameOK = txtName.text.length >= 5;
		txtBirthdate.text = userModel.birthDateFancy(userRow);
		bBirthDateOK = userModel.birthYear(userRow) >= 1940;
		const sex = userModel.sex(userRow);
		chkMale.checked = sex === 0;
		chkFemale.checked = sex === 1;
		bSexOK = sex <= 1;
	}

	function focusOnFirstField() {
		if (!bNameOK)
			txtName.forceActiveFocus();
		else if (!bBirthDateOK)
			caldlg.open();
		else
			frmSex.forceActiveFocus();
	}
}
