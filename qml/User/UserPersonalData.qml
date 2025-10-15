import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"
import "../Pages"

ColumnLayout {
	id: userPersonalModule
	spacing: 10

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
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : 0
	}

	TPTextInput {
		id: txtName
		readOnly: userRow !== 0
		heightAdjustable: false
		ToolTip.text: qsTr("The name is too short")
		Layout.preferredWidth: parent.width*0.9

		property bool bTextChanged: false

		onEnterOrReturnKeyPressed: passwordControl.forceActiveFocus();

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

	Item {
		Layout.fillWidth: true
		Layout.minimumHeight: btnChangePassword.height

		TPButton {
			id: btnChangePassword
			text: qsTr("Change password")
			rounded: false
			imageSource: "password"
			visible: userRow === 0 && userModel.mainUserConfigured
			autoSize: true
			anchors.horizontalCenter: parent.horizontalCenter

			onClicked: changePasswordLoader.active = true;
		}
	}

	TPPassword {
		id: passwordControl
		enabled: bNameOK
		visible: userRow === 0 && !userModel.mainUserConfigured
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5

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
			parentPage: userPersonalModule.parentPage
		}

		onLoaded: item.show(-1);
	}

	TPLabel {
		id: lblBirthdate
		text: userModel.birthdayLabel
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 15 : -5
	}

	TPTextInput {
		id: txtBirthdate
		readOnly: true
		enabled: bPasswordOK
		Layout.minimumWidth: parent.width*0.6
		Layout.maximumWidth: parent.width*0.6

		CalendarDialog {
			id: caldlg
			showDate: userModel.birthDate(userRow)
			initDate: new Date(1940, 0, 1)
			finalDate: new Date()
			parentPage: userPersonalModule.parentPage

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

	Item {
		height: appSettings.itemDefaultHeight
		Layout.fillWidth: true
		Component.onCompleted: Layout.topMargin = (Qt.platform.os !== "android") ? 10 : -5

		TPButtonGroup {
			id: sexGroup
		}

		TPRadioButtonOrCheckBox {
			id: chkMale
			text: qsTr("Male")
			actionable: userRow === 0
			checked: userModel.sex(userRow) === 0
			buttonGroup: sexGroup
			width: parent.width/2

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				if (userModel.sex(userRow) !== 0)
					userModel.setSex(userRow, true);
				bSexOK = true;
			}
		}

		TPRadioButtonOrCheckBox {
			id: chkFemale
			text: qsTr("Female")
			actionable: userRow === 0
			checked: userModel.sex(userRow) === 1
			buttonGroup: sexGroup
			width: parent.width/2

			onClicked: {
				if (userModel.sex(userRow) !== 1)
					userModel.setSex(userRow, false);
				bSexOK = true;
			}

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}
		}
	}

	TPRadioButtonOrCheckBox {
		id: chkOnlineUser
		text: userModel.onlineAccountUserLabel
		actionable: userRow === 0
		radio: false
		checked: userModel.onlineAccount
		Layout.maximumWidth: parent.width * 0.7

		onClicked: userModel.onlineAccount = checked;

		TPButton {
			imageSource: "question.png"
			width: appSettings.itemDefaultHeight
			height: width

			anchors {
				verticalCenter: parent.verticalCenter
				left: parent.right
				leftMargin: -15
			}

			onClicked: showUserRegistrationDialog();
		}
	}

	property TPBalloonTip userRegistrationDlg: null
	function showUserRegistrationDialog(): void {
		if (userRegistrationDlg === null) {
			function createDialog() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					userRegistrationDlg = component.createObject(mainwindow.contentItem, { parentPage: homePage,
					title: qsTr("Online Registration"), button1Text: "OK", button2Text: "",
					message: qsTr("When you register online, you create a unique user account that will enable to sync your workouts and training programs from accross devices.
					You'll be able to do that for your clients as well if you decide to be a trainer or coach.
					You'll get programs and advices from coaches and more.
					But it's not required for the app to work.") });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createDialog();
		}
		userRegistrationDlg.show(-1);
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
		chkOnlineUser.checked = userModel.onlineAccount
		userModel.userPasswordAvailable.connect(getUserPassword);
		userModel.getPassword();
	}

	function getUserPassword(password: string): void {
		passwordControl.setPasswordText(password);
		bPasswordOK = password.length >= 6;
		userModel.userPasswordAvailable.disconnect(getUserPassword);
	}

	function focusOnFirstField(): void {
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
