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
		function onUserNameOK(row: int, b_ok: bool): void {
			if (row === userRow)
				bNameOK = b_ok;
		}
	}

	TPLabel {
		id: lblBirthdate
		text: userModel.birthdayLabel
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
		id: txtBirthdate
		text: userModel.birthDateFancy(userRow)
		readOnly: true
		height: controlsHeight

		Component.onCompleted: bBirthDateOK = userModel.birthYear(userRow) >= 1940;

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

	TPLabel {
		id: lblName
		text: userModel.nameLabel
		height: controlsHeight

		anchors {
			top: txtBirthdate.bottom
			topMargin: 10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	TPTextInput {
		id: txtName
		height: controlsHeight
		text: userModel.userName(userRow);
		enabled: bBirthDateOK
		ToolTip.text: qsTr("The name is too short")

		property bool bTextChanged: false
		Component.onCompleted: bNameOK = userModel.userName(userRow).length >= 5;

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

	Pane {
		id: frmSex
		enabled: bNameOK
		height: controlsHeight
		padding: 0
		spacing: 0

		Component.onCompleted: bSexOK = userModel.sex(userRow) <= 1;

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
			checked: userModel.sex(userRow) === 0
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
			checked: userModel.sex(userRow) === 1
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

	function focusOnFirstField() {
		if (!bNameOK)
			txtName.forceActiveFocus();
		else if (!bBirthDateOK)
			caldlg.open();
		else
			frmSex.forceActiveFocus();
	}
}
