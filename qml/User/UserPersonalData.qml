import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"
import "../Pages"

Frame {
	id: frmUserData
	spacing: controlsSpacing
	padding: 0
	height: minimumHeight
	implicitHeight: height
	implicitWidth: width

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
	}

	required property TPPage parentPage
	required property int userRow
	property bool bReady: bNameOK && bBirthDateOK && bSexOK
	property bool bNameOK: false
	property bool bBirthDateOK: false
	property bool bSexOK: false
	readonly property int nControls: 6
	readonly property int controlsHeight: 30
	readonly property int controlsSpacing: 15
	readonly property int minimumHeight: nControls*controlsHeight

	TPLabel {
		id: lblName
		text: userModel.nameLabel
		height: controlsHeight

		anchors {
			top: parent.top
			topMargin: (availableHeight - minimumHeight)/2
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
		ToolTip.text: qsTr("The name is too short")

		Component.onCompleted: bNameOK = userModel.userName(userRow).length >= 5;

		onEditingFinished: {
			if (bNameOK)
				userModel.setUserName(userRow, text);
		}

		onEnterOrReturnKeyPressed: {
			if (bNameOK)
				caldlg.open();
		}

		onTextEdited: {
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
			topMargin: controlsSpacing
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
		enabled: bNameOK
		height: controlsHeight

		Component.onCompleted: bBirthDateOK = userModel.birthDate(userRow) !== new Date();

		onTextEdited: {
			frmSex.enabled = acceptableInput;
			ToolTip.visible = !acceptableInput;
		}

		anchors {
			top: lblBirthdate.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: btnBirthDate.width + 5
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

		Component.onCompleted: bSexOK = userModel.sex(userRow) <= 1;

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
			text: qsTr("Male")
			height: controlsHeight
			checked: userModel.sex(userRow) === 0
			width: frmSex.width/2

			onClicked: {
				bSexOK = true;
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
			text: qsTr("Female")
			height: controlsHeight
			checked: userModel.sex(userRow) === 1
			width: frmSex.width/2

			onClicked: {
				bSexOK = true;
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
