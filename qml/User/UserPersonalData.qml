import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
	property bool bReady: bNameOK && bPasswordOK && bBirthDateOK && bSexOK
	property bool bNameOK
	property bool bPasswordOK
	property bool bBirthDateOK
	property bool bSexOK
	readonly property int nControls: userRow === 0 ? 7 : 5
	readonly property int controlsHeight: 25
	readonly property int moduleHeight: nControls*(controlsHeight) + 15

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === userRow && field >= 100)
				getUserInfo();
		}
	}

	onUserRowChanged: getUserInfo();
	Component.onCompleted: getUserInfo();

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
		readOnly: userRow !== 0
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

	TPButton {
		id: btnChangePassword
		text: qsTr("Change password")
		imageSource: "password"
		flat: false
		fixedSize: true
		width: parent.width*0.7
		visible: userRow === 0 && userModel.mainUserConfigured

		anchors {
			top: txtName.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}

		onClicked: changePasswordLoader.active = true;
	}

	TPPassword {
		id: passwordControl
		enabled: bNameOK
		visible: userRow === 0 && !userModel.mainUserConfigured

		anchors {
			top: txtName.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		onPasswordUnacceptable: bPasswordOK = false;
		onPasswordAccepted: {
			bPasswordOK = true;
			userModel.setPassword(getPassword());
		}
	}

	Loader {
		id: changePasswordLoader
		active: false
		asynchronous: true
		sourceComponent: UserChangePassword {
			parentPage: frmUserData.parentPage
		}

		onLoaded: item.show(-1);
	}

	TPLabel {
		id: lblBirthdate
		text: userModel.birthdayLabel
		height: controlsHeight

		anchors {
			top: userRow === 0 ? userModel.mainUserConfigured ? btnChangePassword.bottom : passwordControl.bottom : txtName.bottom
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
		enabled: bPasswordOK

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
			enabled: bPasswordOK && userRow === 0
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
			top: txtBirthdate.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		TPRadioButton {
			id: chkMale
			text: qsTr("Male")
			actionable: userRow === 0
			height: controlsHeight
			width: frmSex.width/2

			onClicked: {
				bSexOK = true;
				if (chkFemale.checked) {
					chkFemale.checked = false
					userModel.setSex(userRow, 0);
					userModel.setAvatar(userRow, userModel.defaultAvatar());
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
			actionable: userRow === 0
			height: controlsHeight
			width: frmSex.width/2

			onClicked: {
				bSexOK = true;
				if (chkMale.checked) {
					chkMale.checked = false;
					userModel.setSex(userRow, 1);
					userModel.setAvatar(userRow, userModel.defaultAvatar());
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
		if (userRow === -1)
			return;
		txtName.text = userModel.userName(userRow);
		bNameOK = txtName.text.length >= 5;
		txtBirthdate.text = userModel.birthDateFancy(userRow);
		bBirthDateOK = userModel.birthYear(userRow) >= 1940;
		const sex = userModel.sex(userRow);
		chkMale.checked = sex === 0;
		chkFemale.checked = sex === 1;
		bSexOK = sex <= 1;
		userModel.userPasswordAvailable.connect(getUserPassword);
		userModel.getPassword();
		bPasswordOK = userModel.mainUserConfigured
	}

	function getUserPassword(password): void {
		passwordControl.setPasswordText(password);
		userModel.userPasswordAvailable.disconnect(getUserPassword);
	}

	function focusOnFirstField() {
		if (!bNameOK)
			txtName.forceActiveFocus();
		else if (!bPasswordOK)
			passwordControl.forceActiveFocus();
		else if (!bBirthDateOK)
			caldlg.open();
		else
			frmSex.forceActiveFocus();
	}
}
