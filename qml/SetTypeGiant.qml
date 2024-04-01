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
	required property string setType

	property var ownerExercise
	signal requestTimerDialogSignal(Item requester, var args)

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString() + "  -  " + mainwindow.setTypesModel[setType].text
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
				onClicked: itemManager.removeSetObject(setNumber, exerciseIdx);
			}
		}

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

		RowLayout {
			Layout.fillWidth: true

			Label {
				id: lblExercise1
				text: tDayModel.exerciseName1(exerciseIdx)
				width: setItem.width/2
				font.bold: true
				wrapMode: Text.WordWrap
				Layout.row: 0
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				MouseArea {
					anchors.fill: parent
					onClicked: ownerExercise.changeExercise1(lblExercise1);
				}
			}

			Label {
				id: lblExercise2
				text: tDayModel.exerciseName2(exerciseIdx)
				width: setItem.width/2
				font.bold: true
				wrapMode: Text.WordWrap
				Layout.row: 0
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				MouseArea {
					anchors.fill: parent
					onClicked: ownerExercise.changeExercise2(lblExercise2);
				}
			}
		}

		GridLayout {
			Layout.fillWidth: true
			Layout.topMargin: 10
			Layout.bottomMargin: 10
			rows: 4
			columns: 2
			columnSpacing: 15
			rowSpacing: 5
			Layout.leftMargin: setItem.width/7

			Label {
				text: qsTr("Reps:")
				Layout.row: 0
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNReps1
				type: SetInputField.Type.RepType
				availableWidth: setItem.width/3
				showLabel: false
				Layout.row: 1
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
				Layout.row: 0
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNReps2
				type: SetInputField.Type.RepType
				availableWidth: setItem.width/3
				showLabel: false
				Layout.row: 1
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
				Layout.row: 2
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNWeight1
				text: strWeight1
				type: SetInputField.Type.WeightType
				availableWidth: setItem.width/3
				showLabel: false
				Layout.row: 3
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
				Layout.row: 2
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNWeight2
				type: SetInputField.Type.WeightType
				availableWidth: setItem.width/3
				showLabel: false
				Layout.row: 3
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter

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
