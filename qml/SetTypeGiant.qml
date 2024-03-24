import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

Item {
	id: setItem
	implicitHeight: setLayout.implicitHeight
	Layout.fillWidth: true
	Layout.leftMargin: 5
	Layout.rightMargin: 5

	required property DBTrainingDayModel tDayModel
	required property int exerciseIdx
	required property int setNumber

	property var nextObject: null
	signal requestTimerDialogSignal(Item requester, var args)
	signal requestSimpleExercisesListFromSet(Item setrequester, var bVisible, int id)

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString() + qsTr("  -  Giant set")
			font.bold: true

			RoundButton {
				id: btnRemoveSet
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.right
				height: 25
				width: 25

				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}
				onClicked: {
					appDB.removeSetObject(setNumber, exerciseIdx);
				}
			}
		}

		TextField {
			id: txtExerciseName2
			font.bold: true
			font.pixelSize: AppSettings.fontSizeText
			readOnly: true
			wrapMode: Text.WordWrap
			width: windowWidth - 75
			Layout.minimumWidth: width
			height: 60
			Layout.leftMargin: 5
			Layout.rightMargin: 5
			Layout.topMargin: 0
			visible: setNumber === 0
			z: 1

			background: Rectangle {
				color: txtExerciseName2.readOnly ? "transparent" : "white"
				border.color: txtExerciseName2.readOnly ? "transparent" : "black"
				radius: 5
			}

			Keys.onReturnPressed: { //Alphanumeric keyboard
				btnEditExercise2.clicked();
				txtRestTime.forceActiveFocus();
			}

			Component.onCompleted: {
				text = tDayModel.exerciseName2(exerciseIdx);
				if (text.length === 0)
					text = qsTr("Add exercise...");
			}

			onReadOnlyChanged: {
				if (!readOnly) {
					const idx = text.indexOf(':'); //Remove the '2: ' from the name
					text = text.substring(idx + 1, text.length).trim();
					cursorPosition = text.length;
				}
				else {
					cursorPosition = 0;
					ensureVisible(0);
				}
				requestSimpleExercisesListFromSet(setItem, !readOnly, 1);
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
					cursorPosition = 0;
				}
			}

			onEditingFinished: tDayModel.setExerciseName2(text, exerciseIdx);

			RoundButton {
				id: btnRemoveExercise2
				anchors.left: txtExerciseName2.right
				anchors.verticalCenter: txtExerciseName2.verticalCenter
				height: 25
				width: 25

				z: 2
				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.fill: parent
				}

				onClicked: {
					txtExerciseName2.text = qsTr("Add exercise...");
					tDayModel.setExerciseName2(txtExerciseName2.text, exerciseIdx);
				}
			} //btnRemoveExercise2

			RoundButton {
				id: btnEditExercise2
				anchors.left: btnRemoveExercise2.right
				anchors.verticalCenter: txtExerciseName2.verticalCenter
				height: 25
				width: 25
				z: 2

				Image {
					source: "qrc:/images/"+darkIconFolder+"edit.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}

				onClicked: txtExerciseName2.readOnly = !txtExerciseName2.readOnly;
			} //btnEditExercise2

			RoundButton {
				id: btnClearText
				anchors.left: btnEditExercise.left
				anchors.top: btnEditExercise.bottom
				height: 20
				width: 20
				visible: !txtExerciseName.readOnly

				Image {
					source: "qrc:/images/"+darkIconFolder+"edit-clear.png"
					anchors.fill: parent
					height: 20
					width: 20
				}
				onClicked: {
					txtExerciseName2.clear();
					txtExerciseName2.forceActiveFocus();
				}
			}
		} //txtExerciseName2

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			availableWidth: setItem.width
			windowTitle: lblSetNumber.text
			visible: setNumber > 0

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, str, exerciseIdx);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setRestTime(setNumber, exerciseIdx);
			onEnterOrReturnKeyPressed: txtNReps1.forceActiveFocus();
		}

		GridLayout {
			Layout.fillWidth: true
			Layout.topMargin: 10
			Layout.bottomMargin: 10
			rows: 5
			columns: 2
			columnSpacing: 15
			rowSpacing: 5
			Layout.leftMargin: setItem.width/7

			Label {
				id: lblExercise1
				text: qsTr("Exercise 1")
				width: setItem.width/2
				font.bold: true
				Layout.row: 0
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
			}

			Label {
				id: lblExercise2
				text: qsTr("Exercise 2")
				width: setItem.width/2
				font.bold: true
				Layout.row: 0
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
			}

			Label {
				text: qsTr("Reps:")
				Layout.row: 1
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNReps1
				type: SetInputField.Type.RepType
				availableWidth: setItem.width/3
				showLabel: false
				Layout.row: 2
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, 0, str, exerciseIdx);
					text = str;
				}

				Component.onCompleted: text = tDayModel.setReps(setNumber, 0, exerciseIdx);
				onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();
			}

			Label {
				text: qsTr("Reps:")
				Layout.row: 1
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNReps2
				type: SetInputField.Type.RepType
				availableWidth: setItem.width/3
				showLabel: false
				Layout.row: 2
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, 1, str, exerciseIdx);
					text = str;
				}

				Component.onCompleted: text = tDayModel.setReps(setNumber, 1, exerciseIdx);
				onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
			}

			Label {
				text: qsTr("Weight") + AppSettings.weightUnit + ":"
				Layout.row: 3
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNWeight1
				text: strWeight1
				type: SetInputField.Type.WeightType
				availableWidth: setItem.width/3
				showLabel: false
				Layout.row: 4
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter

				onValueChanged: (str) => {
					tDayModel.setSetWeight(setNumber, 0, str, exerciseIdx);
					text = str;
				}

				Component.onCompleted: text = tDayModel.setWeight(setNumber, 0, exerciseIdx);
				onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();
			}

			Label {
				text: qsTr("Weight") + AppSettings.weightUnit + ":"
				Layout.row: 3
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNWeight2
				type: SetInputField.Type.WeightType
				availableWidth: setItem.width/3
				showLabel: false
				Layout.row: 4
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter

				onEnterOrReturnKeyPressed: {
					if (nextObject !== null)
						nextObject.forceActiveFocus()
				}

				onValueChanged: (str) => {
					tDayModel.setSetWeight(setNumber, 1, str, exerciseIdx);
					text = str;
				}

				Component.onCompleted: text = tDayModel.setWeight(setNumber, 1, exerciseIdx);
			}
		}

		Label {
			text: qsTr("Notes:")
			Layout.bottomMargin: 10
			font.bold: true

			RoundButton {
				id: btnShowHideNotes
				anchors.left: parent.right
				anchors.verticalCenter: parent.verticalCenter
				anchors.rightMargin: 20
				width: 25
				height: 25

				Image {
					id: img
					source: "qrc:/images/"+darkIconFolder+"down.png"
					width: 20
					height: 20
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
				}

				onClicked: {
					txtSetNotes.visible = !txtSetNotes.visible;
					img.source = txtSetNotes.visible ? "qrc:/images/"+darkIconFolder+"up.png" : "qrc:/images/"+darkIconFolder+"down.png"
				}
			}
		}
		TextField {
			id: txtSetNotes
			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 5
			Layout.bottomMargin: 10
			visible: false

			onTextEdited: tDayModel.setSetNotes(text, exerciseIdx);
			Component.onCompleted: text = tDayModel.setNotes(setNumber, exerciseIdx);
		}
	} //ColumnLayout setLayout

	function changeExercise(newname)
	{
		txtExerciseName2.text = newname;
		tDayModel.setExerciseName2(newname, exerciseIdx);
	}

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}
} // FocusScope
