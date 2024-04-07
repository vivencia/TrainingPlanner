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
			text: qsTr("Set #") + (setNumber + 1).toString()
			font.bold: true

			TPComboBox {
				id: cboSetType
				model: mainwindow.setTypesModel
				currentIndex: setType
				anchors.left: parent.right
				anchors.leftMargin: 10
				anchors.verticalCenter: parent.verticalCenter
				width: 120

				onActivated: (index)=> {
					if (index !== setType)
						itemManager.changeSetType(setNumber, exerciseIdx, index);
				}
			}

			RoundButton {
				id: btnRemoveSet
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: cboSetType.right
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
				tDayModel.setSetRestTime(setNumber, exerciseIdx, str);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setRestTime(setNumber, exerciseIdx);
			onEnterOrReturnKeyPressed: txtNReps1.forceActiveFocus();
		}

		RowLayout {
			Layout.fillWidth: true

			Label {
				id: lblExercise1
				objectName: "lblExercise1"
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
					onClicked: ownerExercise.changeExercise1();
				}
			}

			Label {
				id: lblExercise2
				objectName: "lblExercise2"
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
					onClicked: ownerExercise.changeExercise2();
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
					tDayModel.setSetReps(setNumber, exerciseIdx, 0, str);
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
					tDayModel.setSetReps(setNumber, exerciseIdx, 1, str);
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
					tDayModel.setSetWeight(setNumber, exerciseIdx, 0, str);
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
					tDayModel.setSetWeight(setNumber, exerciseIdx, 1, str);
					text = str;
				}

				Component.onCompleted: text = tDayModel.setWeight(setNumber, 1, exerciseIdx);
			}
		}

		SetNotesField {
			id: btnShowHideNotes
			Layout.fillWidth: true
		}
	} //ColumnLayout setLayout

	function changeLabel(labelObj, newtext)
	{
		labelObj.text = newtext;
	}

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}
} // FocusScope
