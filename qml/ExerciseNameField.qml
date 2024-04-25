import QtQuick
import QtQuick.Controls

TextField {
	id: control
	font.bold: true
	font.pointSize: AppSettings.fontSizeText
	readOnly: true
	wrapMode: Text.WordWrap
	height: 60
	leftInset: 5
	rightInset: 5
	padding: 0
	z: 1

	property bool showRemoveButton: true

	signal exerciseChanged(string new_exercise)
	signal removeButtonClicked()
	signal editButtonClicked()

	background: Rectangle {
		color: readOnly ? "transparent" : AppSettings.fontColor
		border.color: readOnly ? "transparent" : "black"
		radius: 5
	}

	onTextChanged: {
		if (readOnly)
			ensureVisible(0);
	}

	onActiveFocusChanged: {
		if (activeFocus)
			cursorPosition = text.length;
		else {
			readOnly = false;
			exerciseChanged(text);
			cursorPosition = 0;
			ensureVisible(0);
		}
	}

	RoundButton {
		id: btnRemoveExercise
		height: 25
		width: 25
		padding: 5
		visible: showRemoveButton
		z: 2
		anchors {
			left: control.right
			top: control.top
		}

		Image {
			source: "qrc:/images/"+darkIconFolder+"remove.png"
			asynchronous: true
			anchors.fill: parent
			height: 25
			width: 25
		}

		onClicked: removeButtonClicked();
	} //btnRemoveExercise

	RoundButton {
		id: btnEditExercise
		anchors.left: btnRemoveExercise.right
		anchors.top: control.top
		height: 25
		width: 25
		padding: 5
		z: 2

		Image {
			source: "qrc:/images/"+darkIconFolder+"edit.png"
			asynchronous: true
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
			height: 20
			width: 20
		}

		onClicked: {
			control.readOnly = !control.readOnly;
			editButtonClicked();
		}
	}

	RoundButton {
		id: btnClearText
		anchors.left: btnEditExercise.left
		anchors.top: btnEditExercise.bottom
		height: 20
		width: 20
		visible: !control.readOnly

		Image {
			source: "qrc:/images/"+darkIconFolder+"edit-clear.png"
			asynchronous: true
			anchors.fill: parent
			height: 20
			width: 20
		}
		onClicked: {
			control.clear();
			control.forceActiveFocus();
		}
	}
}
