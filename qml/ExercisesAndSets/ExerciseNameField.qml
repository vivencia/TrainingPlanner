import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Item {
	id: control

	property alias text: txtField.text
	property alias readOnly: txtField.readOnly
	property bool showRemoveButton: true
	property bool bCanEmitTextChanged: false
	property bool bTextChanged: false
	property bool bEditable: true

	signal exerciseChanged(string new_exercise)
	signal removeButtonClicked()
	signal editButtonClicked()
	signal mousePressed(var mouse)
	signal mousePressAndHold(var mouse)
	signal itemClicked()

	TextField {
		id: txtField
		font.bold: true
		font.pixelSize: appSettings.fontSize
		readOnly: true
		wrapMode: Text.WordWrap
		topPadding: 5
		leftPadding: 10
		rightPadding: 10
		bottomPadding: 5
		topInset: 0
		leftInset: 0
		rightInset: 0
		bottomInset: 0
		width: 0.8*control.width

		anchors {
			top: control.top
			left: control.left
			bottom: control.bottom
		}

		background: Rectangle {
			color: control.readOnly ? "transparent" : "white"
			border.color: control.readOnly ? "transparent" : "black"
			radius: 5
		}

		onPressed: (mouse) => mousePressed(mouse);
		onPressAndHold: (mouse) => mousePressAndHold(mouse);

		MouseArea {
			enabled: txtField.readOnly
			anchors.fill: txtField
			onClicked: itemClicked();
		}

		Component.onCompleted: {
			ensureVisible(0);
			cursorPosition = 0;
		}

		onReadOnlyChanged: {
			if (readOnly) {
				ensureVisible(0);
				cursorPosition = 0;
				if (bTextChanged) {
					bTextChanged = false;
					exerciseChanged(text);
				}
			}
			else
				cursorPosition = text.length;
		}

		onEditingFinished: {
			if (bTextChanged) {
				bTextChanged = false;
				exerciseChanged(text);
			}
		}

		onTextChanged: {
			bTextChanged = true;
			if (bCanEmitTextChanged) {
				if (!readOnly) {
					bTextChanged = false;
					exerciseChanged(text);
				}
				else {
					ensureVisible(0);
					cursorPosition = 0;
				}
			}
		}

		TPButton {
			id: btnClearText
			imageSource: "edit-clear"
			visible: !txtField.readOnly
			focus: false

			anchors {
				right: txtField.right
				rightMargin: 5
				verticalCenter: txtField.verticalCenter
			}

			onClicked: {
				txtField.clear();
				txtField.forceActiveFocus();
			}
		}
	}

	TPButton {
		id: btnEditExercise
		imageSource: "black/edit"
		imageSize: 25
		height: 25
		width: 25
		enabled: bEditable

		anchors {
			left: txtField.right
			leftMargin: -10
			verticalCenter: control.verticalCenter
		}

		onClicked: {
			txtField.readOnly = !txtField.readOnly;
			editButtonClicked();
		}
	}

	TPButton {
		id: btnRemoveExercise
		imageSource: "remove"
		imageSize: 25
		height: 25
		width: 25
		visible: showRemoveButton
		enabled: bEditable

		anchors {
			left: btnEditExercise.right
			rightMargin: 5
			verticalCenter: control.verticalCenter
		}

		onClicked: removeButtonClicked();
	} //btnRemoveExercise
}

