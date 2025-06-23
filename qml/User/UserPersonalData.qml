import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"
import "../Pages"

Column {
	id: frmUserData
	spacing: 5
	padding: 5

	required property TPPage parentPage
	required property int userRow
	property bool bReady: bNameOK && bPasswordOK && bBirthDateOK && bSexOK
	property bool bNameOK
	property bool bPasswordOK
	property bool bBirthDateOK
	property bool bSexOK

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
	}

	TPTextInput {
		id: txtName
		readOnly: userRow !== 0
		width: parent.width*0.9
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
	}

	TPButton {
		id: btnChangePassword
		text: qsTr("Change password")
		imageSource: "password"
		flat: false
		width: parent.width*0.7
		height: appSettings.itemDefaultHeight
		visible: userRow === 0 && userModel.mainUserConfigured
		Layout.alignment: Qt.AlignCenter

		onClicked: changePasswordLoader.active = true;
	}

	TPPassword {
		id: passwordControl
		enabled: bNameOK
		visible: userRow === 0 && !userModel.mainUserConfigured

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
	}

	TPTextInput {
		id: txtBirthdate
		readOnly: true
		enabled: bPasswordOK
		width: parent.width*0.6

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
			width: appSettings.itemDefaultHeight
			height: width
			enabled: bPasswordOK && userRow === 0
			anchors {
				left: txtBirthdate.right
				verticalCenter: txtBirthdate.verticalCenter
			}

			onClicked: caldlg.open();
		}
	}

	RowLayout {
		spacing: 10
		width: parent.width

		TPRadioButton {
			id: chkMale
			text: qsTr("Male")
			actionable: userRow === 0
			checked: userModel.sex() === 0
			Layout.preferredWidth: parent.width/2
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				bSexOK = true;
				if (chkFemale.checked) {
					chkFemale.checked = false
					userModel.setSex(userRow, true);
				}
			}
		}

		TPRadioButton {
			id: chkFemale
			text: qsTr("Female")
			actionable: userRow === 0
			checked: userModel.sex() === 1
			Layout.preferredWidth: parent.width/2
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				bSexOK = true;
				if (chkMale.checked) {
					chkMale.checked = false;
					userModel.setSex(userRow, false);
				}
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
