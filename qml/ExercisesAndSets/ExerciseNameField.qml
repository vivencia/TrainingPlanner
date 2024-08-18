import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

RowLayout {
	spacing: 5
	height: 60
	z: 0

	property alias text: control.text
	property alias readOnly: control.readOnly
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
		id: control
		font.bold: true
		font.pointSize: AppSettings.fontSize
		readOnly: true
		wrapMode: Text.WordWrap
		z: 1
		width: parent.width-(showRemoveButton ? 50 : 25)
		leftPadding: 0
		rightPadding: 0
		height: parent.height
		Layout.maximumWidth: width
		Layout.minimumWidth: width
		Layout.topMargin: 0

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

	TPButton {
		id: btnRemoveExercise
		imageSource: "qrc:/images/remove.png"
		backgroundColor: "transparent"
		height: 20
		width: 20
		visible: showRemoveButton
		enabled: bEditable
		Layout.leftMargin: -10
		z: 1

		onClicked: removeButtonClicked();
	} //btnRemoveExercise

	TPButton {
		id: btnEditExercise
		imageSource: "edit.png"
		backgroundColor: "transparent"
		height: 20
		width: 20
		enabled: bEditable
		z: 1
		Layout.leftMargin: -10

		onClicked: {
			control.readOnly = !control.readOnly;
			editButtonClicked();
		}
	}
}
