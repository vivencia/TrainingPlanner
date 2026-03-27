import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

Item {
	id: _control

//public:
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
	    font.pixelSize: AppSettings.fontSize
		color: readOnly ? AppSettings.fontColor : "black"
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
		width: 0.75*_control.width
		height: _control.height

		property bool text_edited: false

		anchors {
			top: _control.top
			left: _control.left
			bottom: _control.bottom
		}

		background: Rectangle {
			color: _control.readOnly ? "transparent" : "white"
			border.color: _control.readOnly ? "transparent" : AppSettings.fontColor
			radius: 5
		}

		MouseArea {
			enabled: txtField.enabled && txtField.readOnly
			anchors.fill: txtField
			onClicked: _control.itemClicked();
			onPressAndHold: _control.itemPressed();
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
				_control.exerciseChanged(text);
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
		width: AppSettings.itemDefaultHeight
		height: width
		visible: _control.showEditButton
		enabled: _control.editable

		anchors {
			left: txtField.right
			leftMargin: 5
			verticalCenter: _control.verticalCenter
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
		width: AppSettings.itemDefaultHeight
		height: width
		visible: _control.showExercisesListButton
		enabled: _control.editable

		anchors {
			left: btnEditExercise.right
			leftMargin: 5
			verticalCenter: _control.verticalCenter
		}

		onClicked: _control.showExercisesListButtonClicked();
	}

	TPButton {
		id: btnRemoveExercise
		imageSource: "remove"
		width: AppSettings.itemDefaultHeight
		height: width
		visible: _control.showRemoveButton
		enabled: _control.editable

		anchors {
			left: _control.showExercisesListButton ? btnShowList.right :
																(_control.showEditButton ? btnEditExercise.right : txtField.right)
			leftMargin: 5
			verticalCenter: _control.verticalCenter
		}

		onClicked: _control.removeButtonClicked();
	} //btnRemoveExercise
}

