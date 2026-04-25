pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Pages
import TpQml.User
import TpQml.Dialogs

ColumnLayout {
	id: userPersonalModule
	spacing: 10

//public:
	required property TPPage parentPage
	required property int userRow
	property bool bReady: bNameOK && bPasswordOK && bBirthDateOK && bSexOK

//private:
	property bool bNameOK
	property bool bPasswordOK
	property bool bBirthDateOK
	property bool bSexOK

	Connections {
		target: AppUserModel
		function onUserModified(row: int, field: int): void {
			if (row === userPersonalModule.userRow && field >= 100)
				userPersonalModule.getUserInfo();
		}
	}

	onUserRowChanged: getUserInfo();
	Component.onCompleted: getUserInfo();

	TPLabel {
		id: lblName
		text: AppUserModel.nameLabel
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 0
	}

	TPTextInput {
		id: txtName
		readOnly: userPersonalModule.userRow !== 0
		heightAdjustable: false
		ToolTip.text: qsTr("The name is too short")
		Layout.preferredWidth: parent.width * 0.9

		property bool bTextChanged: false

		onEnterOrReturnKeyPressed: passwordControl.forceActiveFocus();

		onEditingFinished: {
			if (bTextChanged && userPersonalModule.bNameOK) {
				AppUserModel.setUserName(userPersonalModule.userRow, text);
				bTextChanged = false;
			}
		}

		onTextEdited: {
			bTextChanged = true;
			if (text.length >= 5) {
				ToolTip.visible = false;
				userPersonalModule.bNameOK = true;
			}
			else {
				ToolTip.visible = true;
				userPersonalModule.bNameOK = false;
			}
		}

		TPButton {
			imageSource: "chat_"
			width: AppSettings.itemDefaultHeight
			height: width
			visible: userPersonalModule.userRow != 0 && AppUserModel.onlineAccount
			enabled: userPersonalModule.bNameOK

			anchors {
				left: txtName.right
				leftMargin: 5
				verticalCenter: txtName.verticalCenter
			}

			onClicked: AppMessages.openChat(AppUserModel.userName(userPersonalModule.userRow));
		}
	}

	Item {
		Layout.fillWidth: true
		Layout.minimumHeight: btnChangePassword.height

		TPButton {
			id: btnChangePassword
			text: qsTr("Change password")
			rounded: false
			imageSource: "password"
			visible: userPersonalModule.userRow === 0 && AppUserModel.mainUserConfigured
			autoSize: true
			anchors.horizontalCenter: parent.horizontalCenter

			onClicked: changePasswordLoader.active = true;
		}
	}

	TPPassword {
		id: passwordControl
		enabled: userPersonalModule.bNameOK
		visible: userPersonalModule.userRow === 0 && !AppUserModel.mainUserConfigured
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5

		onPasswordUnacceptable: userPersonalModule.bPasswordOK = false;
		onPasswordAccepted: {
			userPersonalModule.bPasswordOK = true;
			AppUserModel.setPassword(getPassword());
		}
	}

	Loader {
		id: changePasswordLoader
		active: false
		asynchronous: true

		property UserChangePassword _change_passwd
		sourceComponent: UserChangePassword {
			parentPage: userPersonalModule.parentPage
			Component.onCompleted: changePasswordLoader._change_passwd = this;
		}

		onLoaded: _change_passwd.tpOpen();
	}

	TPLabel {
		id: lblBirthdate
		text: AppUserModel.birthdayLabel
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 15 : -5
	}

	TPTextInput {
		id: txtBirthdate
		readOnly: true
		enabled: userPersonalModule.bPasswordOK
		Layout.minimumWidth: userPersonalModule.width * 0.6
		Layout.maximumWidth: userPersonalModule.width * 0.6

		CalendarDialog {
			id: caldlg
			showDate: AppUserModel.birthDate(userPersonalModule.userRow)
			initDate: new Date(1940, 0, 1)
			finalDate: new Date()
			parentPage: userPersonalModule.parentPage

			onDateSelected: (date) => {
				AppUserModel.setBirthDate(userPersonalModule.userRow, date);
				txtBirthdate.text = AppUserModel.birthDateFancy(userPersonalModule.userRow);
				userPersonalModule.bBirthDateOK = true;
				if (txtName.text.length === 0)
					txtName.forceActiveFocus();
			}
		}

		TPButton {
			id: btnBirthDate
			imageSource: "calendar.png"
			width: AppSettings.itemDefaultHeight
			height: width
			enabled: userPersonalModule.bPasswordOK && userPersonalModule.userRow === 0

			anchors {
				left: txtBirthdate.right
				verticalCenter: txtBirthdate.verticalCenter
			}

			onClicked: caldlg.open();
		}
	}

	Item {
		Layout.preferredHeight: AppSettings.itemDefaultHeight
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5

		TPButtonGroup {
			id: sexGroup
		}

		TPRadioButtonOrCheckBox {
			id: chkMale
			text: qsTr("Male")
			actionable: userPersonalModule.userRow === 0
			checked: AppUserModel.sex(userPersonalModule.userRow) === 0
			buttonGroup: sexGroup
			width: parent.width/2

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				if (AppUserModel.sex(userPersonalModule.userRow) !== 0)
					AppUserModel.setSex(userPersonalModule.userRow, true);
				userPersonalModule.bSexOK = true;
			}
		}

		TPRadioButtonOrCheckBox {
			id: chkFemale
			text: qsTr("Female")
			actionable: userPersonalModule.userRow === 0
			checked: AppUserModel.sex(userPersonalModule.userRow) === 1
			buttonGroup: sexGroup
			width: parent.width / 2

			onClicked: {
				if (AppUserModel.sex(userPersonalModule.userRow) !== 1)
					AppUserModel.setSex(userPersonalModule.userRow, false);
				userPersonalModule.bSexOK = true;
			}

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}
		}
	}

	TPRadioButtonOrCheckBox {
		id: chkOnlineUser
		text: AppUserModel.onlineAccountUserLabel
		actionable: userPersonalModule.userRow === 0
		radio: false
		checked: AppUserModel.onlineAccount
		Layout.maximumWidth: userPersonalModule.width * 0.7

		onClicked: AppUserModel.onlineAccount = checked;

		TPButton {
			imageSource: "question.png"
			width: AppSettings.itemDefaultHeight
			height: width

			anchors {
				verticalCenter: parent.verticalCenter
				left: parent.right
				leftMargin: -15
			}

			onClicked: userPersonalModule.showUserRegistrationDialog();
		}
	}

	Loader {
		id: userRegistrationDlgLoader
		asynchronous: true
		active: false

		property TPBalloonTip _registration_dlg
		sourceComponent: TPBalloonTip {
			parentPage: userPersonalModule.parentPage
			title: qsTr("Online Registration")
			button1Text: "OK"
			button2Text: ""
			message: qsTr(`When you register online, you create a unique user account that will enable to sync your workouts and training programs from accross devices.
						  You'll be able to do that for your clients as well if you decide to be a trainer or coach.
						  You'll get programs and advices from coaches and more.
						  But it's not required for the app to work.`)

			onClosed: userRegistrationDlgLoader.active = false;
			Component.onCompleted: userRegistrationDlgLoader._registration_dlg = this;
		}

		onLoaded: _registration_dlg.tpOpen();
	}
	function showUserRegistrationDialog(): void {
		userRegistrationDlgLoader.active = true;
	}

	function getUserInfo(): void {
		if (userPersonalModule.userRow === -1)
			return;
		txtName.text = AppUserModel.userName(userPersonalModule.userRow);
		bNameOK = txtName.text.length >= 5;
		txtBirthdate.text = AppUserModel.birthDateFancy(userPersonalModule.userRow);
		bBirthDateOK = AppUserModel.birthYear(userPersonalModule.userRow) >= 1940;
		const sex = AppUserModel.sex(userPersonalModule.userRow);
		chkMale.checked = sex === 0;
		chkFemale.checked = sex === 1;
		bSexOK = sex <= 1;
		chkOnlineUser.checked = AppUserModel.onlineAccount
		AppUserModel.userPasswordAvailable.connect(getUserPassword);
		AppUserModel.getPassword();
	}

	function getUserPassword(password: string): void {
		passwordControl.setPasswordText(password);
		bPasswordOK = password.length >= 6;
		AppUserModel.userPasswordAvailable.disconnect(getUserPassword);
	}

	function focusOnFirstField(): void {
		if (!bNameOK)
			txtName.forceActiveFocus();
		else if (!bPasswordOK)
			passwordControl.forceActiveFocus();
		else if (!bBirthDateOK)
			caldlg.open();
		else
			chkMale.forceActiveFocus();
	}
}
