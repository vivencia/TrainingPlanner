import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Row {
	spacing: 0
	height: 60
	z: 0

	property alias text: control.text
	property alias readOnly: control.readOnly
	property bool showRemoveButton: true

	signal exerciseChanged(string new_exercise)
	signal removeButtonClicked()
	signal editButtonClicked()
	signal mousePressed(var mouse)
	signal mousePressAndHold(var mouse)
	signal itemClicked()

	TextField {
		id: control
		font.bold: true
		font.pointSize: AppSettings.fontSizeText
		readOnly: true
		wrapMode: Text.WordWrap
		rightPadding: 20
		z: 1
		width: parent.width-(showRemoveButton ? 50 : 25)
		Layout.fillHeight: true
		Layout.maximumWidth: width
		Layout.minimumWidth: width

		background: Rectangle {
			color: control.readOnly ? "transparent" : "white"
			border.color: control.readOnly ? "transparent" : "black"
			radius: 5
		}

		onPressed: (mouse) => mousePressed(mouse);
		onPressAndHold: (mouse) => mousePressAndHold(mouse);

		MouseArea {
			anchors.fill: control
			enabled: control.readOnly
			z:2
			onClicked: itemClicked();
		}

		onReadOnlyChanged: {
			if (readOnly) {
				ensureVisible(0);
				cursorPosition = 0;
			}
			else
				cursorPosition = text.length;
		}

		onTextChanged: {
			ensureVisible(0);
			cursorPosition = 0;
		}

		onEditingFinished: exerciseChanged(text);

		TPRoundButton {
			id: btnClearText
			height: 20
			width: 20
			visible: !control.readOnly
			imageName: "edit-clear.png"
			anchors {
				right: control.right
				rightMargin: 5
				verticalCenter: control.verticalCenter
			}

			onClicked: {
				control.clear();
				control.forceActiveFocus();
			}
		}
	}

	TPRoundButton {
		id: btnRemoveExercise
		height: 25
		width: 25
		padding: 5
		visible: showRemoveButton
		z: 1
		imageName: "remove.png"
		onClicked: removeButtonClicked();
	} //btnRemoveExercise

	TPRoundButton {
		id: btnEditExercise
		height: 25
		width: 25
		padding: 5
		z: 1
		imageName: "edit.png"

		onClicked: {
			control.readOnly = !control.readOnly;
			editButtonClicked();
		}
	}
}
