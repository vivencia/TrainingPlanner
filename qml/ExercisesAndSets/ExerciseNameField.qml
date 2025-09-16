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
	property bool showEditButton: true
	property bool showExercisesListButton: true
	property bool editable: true

	signal exerciseChanged(string new_exercise)
	signal showExercisesListButtonClicked()
	signal removeButtonClicked()
	signal itemClicked()
	signal itemPressed()

	TextField {
		id: txtField
		font.weight: Font.Bold
	    font.hintingPreference: Font.PreferFullHinting
	    font.pixelSize: userSettings.fontSize
		color: readOnly ? userSettings.fontColor : "black"
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

		property bool text_edited: false

		anchors {
			top: control.top
			left: control.left
			bottom: control.bottom
		}

		background: Rectangle {
			color: control.readOnly ? "transparent" : "white"
			border.color: control.readOnly ? "transparent" : userSettings.fontColor
			radius: 5
		}

		MouseArea {
			enabled: txtField.enabled && txtField.readOnly
			anchors.fill: txtField
			onClicked: itemClicked();
			onPressAndHold: itemPressed();
		}

		Component.onCompleted: {
			ensureVisible(0);
			cursorPosition = 0;
		}

		onReadOnlyChanged: {
			if (readOnly) {
				ensureVisible(0);
				cursorPosition = 0;
			}
			else
				cursorPosition = text.length;
		}

		onTextEdited: text_edited = true;

		onEditingFinished: {
			if (text_edited) {
				text_edited = false;
				exerciseChanged(text);
			}
		}

		TPButton {
			id: btnClearText
			imageSource: "edit-clear"
			hasDropShadow: false
			width: 20
			height: 20
			visible: !txtField.readOnly
			focus: false

			anchors {
				right: txtField.right
				rightMargin: 10
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
		width: userSettings.itemDefaultHeight
		height: width
		visible: showEditButton
		enabled: editable

		anchors {
			left: txtField.right
			leftMargin: 5
			verticalCenter: control.verticalCenter
		}

		onClicked: {
			txtField.readOnly = !txtField.readOnly;
			if (!txtField.readOnly)
				txtField.forceActiveFocus();
		}
	}

	TPButton {
		id: btnShowList
		imageSource: "list.png"
		width: userSettings.itemDefaultHeight
		height: width
		visible: showExercisesListButton
		enabled: editable

		anchors {
			left: btnEditExercise.right
			leftMargin: 5
			verticalCenter: control.verticalCenter
		}

		onClicked: showExercisesListButtonClicked();
	}

	TPButton {
		id: btnRemoveExercise
		imageSource: "remove"
		width: userSettings.itemDefaultHeight
		height: width
		visible: showRemoveButton
		enabled: editable

		anchors {
			left: showExercisesListButton ? btnShowList.right : (showEditButton ? btnEditExercise.right : txtField.right)
			leftMargin: 5
			verticalCenter: control.verticalCenter
		}

		onClicked: removeButtonClicked();
	} //btnRemoveExercise
}

