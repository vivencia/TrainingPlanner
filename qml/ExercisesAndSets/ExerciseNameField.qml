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
	property bool editable: true

	signal exerciseChanged(string new_exercise)
	signal removeButtonClicked()
	signal editButtonClicked()
	signal itemClicked()
	signal itemPressed()

	TextField {
		id: txtField
		font.weight: Font.Bold
	    font.hintingPreference: Font.PreferFullHinting
	    font.pixelSize: appSettings.fontSize
	    color: readOnly ? "transparent" : "black"
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
		width: 0.75*control.width
		height: control.height

		anchors {
			top: control.top
			left: control.left
			bottom: control.bottom
		}

		background: Rectangle {
			color: control.readOnly ? "transparent" : "white"
			border.color: control.readOnly ? "transparent" : appSettings.fontColor
			radius: 5
		}

		MouseArea {
			enabled: txtField.readOnly
			anchors.fill: txtField
			onClicked: itemClicked();
			onPressAndHold: itemPressed();
		}

		TPLabel {
			id: readOnlyText
			text: parent.text
			wrapMode: Text.WordWrap
			visible: parent.readOnly
			leftPadding: 10
			width: parent.width
			height: parent.height
			x: 0
			y: 0
		}

		Component.onCompleted: {
			ensureVisible(0);
			cursorPosition = 0;
		}

		onReadOnlyChanged: {
			if (readOnly) {
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
			hasDropShadow: false
			imageSize: 20
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
		imageSource: "edit.png"
		imageSize: 25
		height: 25
		width: 25
		enabled: editable

		anchors {
			left: txtField.right
			leftMargin: 5
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
		enabled: editable

		anchors {
			left: btnEditExercise.right
			leftMargin: 5
			verticalCenter: control.verticalCenter
		}

		onClicked: removeButtonClicked();
	} //btnRemoveExercise
}

