import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import ".."
import "../TPWidgets"
import "../Dialogs"
import "../Pages"

Frame {
	id: frmImport
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

	TPRadioButton {
		id: optNewUser
		text: qsTr("Create a new user")
		multiLine: true
		height: itemHeight
	}

	TPRadioButton {
		id: optImportUser
		text: qsTr("User already registered")
		multiLine: true
		height: itemHeight
	}
}
