import QtQuick
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"

Frame {
	spacing: controlsSpacing
	padding: 0
	implicitHeight: allControlsHeight + controlsSpacing

	property bool bReady: bNameOK && bBirthDateOK && bSexOK
	property bool bNameOK: false
	property bool bBirthDateOK: false
	property bool bSexOK: false
	readonly property int nControls: 5
	readonly property int controlsHeight: 30
	readonly property int allControlsHeight: nControls*controlsHeight
	readonly property int controlsSpacing: 10

	Label {
		id: lblName
		text: userModel.columnLabel(1)
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		height: controlsHeight
		padding: 0
		bottomInset: 0
		topInset: 0
		bottomPadding: 0

		anchors {
			top: parent.top
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

		Component.onCompleted: {
			text = userModel.userName;
			bNameOK = !userModel.isEmpty();
		}

		onTextChanged: userModel.userName = text;

		onEnterOrReturnKeyPressed: {
			if (bNameOK)
				btnBirthDate.clicked();
		}

		onTextEdited: {
			if (text.length >=5) {
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
			topMargin: -10
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
		height: controlsHeight
		padding: 0

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
		text: runCmd.formatDate(userModel.birthDate)
		readOnly: true
		enabled: bNameOK
		height: controlsHeight

		Component.onCompleted: bBirthDateOK = !userModel.isEmpty();

		onTextEdited: {
			frmSex.enabled = acceptableInput;
			ToolTip.visible = !acceptableInput;
		}

		anchors {
			top: lblBirthdate.bottom
			topMargin: -10
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: btnBirthDate.width + 5
		}

		CalendarDialog {
			id: caldlg
			showDate: userModel.birthDate
			initDate: new Date(1940, 0, 1)
			finalDate: new Date()
			parentPage: homePage

			onDateSelected: (date) => {
				userModel.birthDate = date;
				bBirthDateOK = true;
			}
		}

		TPRoundButton {
			id: btnBirthDate
			width: 25
			height: controlsHeight
			imageName: "calendar.png"

			onClicked: caldlg.open();

			anchors.left: txtBirthdate.right
			anchors.verticalCenter: txtBirthdate.verticalCenter
		}
	}

	Pane {
		id: frmSex
		enabled: bBirthDateOK
		height: controlsHeight
		padding: 0
		spacing: 0

		Component.onCompleted: bSexOK = !userModel.isEmpty();

		background: Rectangle {
			color: "transparent"
		}

		anchors {
			top: txtBirthdate.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		TPRadioButton {
			text: qsTr("Male")
			height: controlsHeight
			checked: userModel.sex === qsTr("Male")

			onCheckedChanged: if (checked) userModel.sex = qsTr("Male");
			onClicked: bSexOK = true

			anchors {
				verticalCenter: parent.verticalCenter
				left: parent.left
				leftMargin: 10
			}
		}

		TPRadioButton {
			text: qsTr("Female")
			height: controlsHeight
			checked: userModel.sex === qsTr("Female")

			onCheckedChanged: if (checked) userModel.sex = qsTr("Female");
			onClicked: bSexOK = true;

			anchors {
				verticalCenter: parent.verticalCenter
				right: parent.right
				rightMargin: 10
			}
		}
	}

	function focusOnFirstField() {
		if (txtName.text.length === 0)
			txtName.forceActiveFocus();
	}
}
