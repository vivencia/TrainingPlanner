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
	required property int setType

	property var ownerExercise
	signal requestTimerDialogSignal(Item requester, var args)

	onFocusChanged: {
		if (focus)
			txtNReps1.forceActiveFocus();
	}

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString()
			font.bold: true
			Layout.bottomMargin: 10

			TPComboBox {
				id: cboSetType
				currentIndex: setType
				anchors {
					left: parent.right
					leftMargin: 10
					verticalCenter: parent.verticalCenter
				}

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
			text: tDayModel.setRestTime(setNumber, exerciseIdx);
			availableWidth: setItem.width
			windowTitle: lblSetNumber.text
			visible: setNumber > 0

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, exerciseIdx, str);
				text = str;
			}

			onEnterOrReturnKeyPressed: txtNReps1.forceActiveFocus();
		}

		RowLayout {
			Layout.fillWidth: true

			ExerciseNameField {
				id: txtExercise1
				objectName: "txtExercise1"
				text: tDayModel.exerciseName1(exerciseIdx)
				showRemoveButton: false
				width: setItem.width/2
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				onExerciseChanged: (new_text) => ownerExercise.changeExercise1(false);
				onEditButtonClicked: ownerExercise.changeExercise1(true);
			}

			ExerciseNameField {
				id: txtExercise2
				objectName: "txtExercise2"
				text: tDayModel.exerciseName2(exerciseIdx)
				showRemoveButton: false
				width: setItem.width/2
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				onExerciseChanged: (new_text) => ownerExercise.changeExercise2(false);
				onEditButtonClicked: ownerExercise.changeExercise2(true);
			}
		}

		RowLayout {
			Layout.fillWidth: true
			spacing: 5

			SetInputField {
				id: txtNReps1
				type: SetInputField.Type.RepType
				text: tDayModel.setReps(setNumber, 0, exerciseIdx);
				availableWidth: !btnCopyValue.visible ? setItem.width/2 + 10 : setItem.width/3
				Layout.alignment: Qt.AlignLeft
				showLabel: !btnCopyValue.visible

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, exerciseIdx, 0, str);
					text = str;
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue.visible = true;
				}

				onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();
			}

			RoundButton {
				id: btnCopyValue
				visible: false
				Layout.alignment: Qt.AlignRight

				Image {
					source: "qrc:/images/"+darkIconFolder+"copy-setvalue.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}

				onClicked: {
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, 0);
					visible = false;
				}
			}

			SetInputField {
				id: txtNReps2
				type: SetInputField.Type.RepType
				text: tDayModel.setReps(setNumber, 1, exerciseIdx);
				availableWidth: setItem.width/4 + 15
				showLabel: false
				Layout.alignment: !btnCopyValue2.visible ? Qt.AlignRight : Qt.AlignLeft

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, exerciseIdx, 1, str);
					text = str;
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue2.visible = true;
				}

				onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
			}

			RoundButton {
				id: btnCopyValue2
				visible: false
				Layout.alignment: Qt.AlignRight

				Image {
					source: "qrc:/images/"+darkIconFolder+"copy-setvalue.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}

				onClicked: {
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, 1);
					visible = false;
				}
			}
		}

		RowLayout {
			Layout.fillWidth: true
			spacing: 5

			SetInputField {
				id: txtNWeight1
				text: tDayModel.setWeight(setNumber, 0, exerciseIdx);
				type: SetInputField.Type.WeightType
				availableWidth: !btnCopyValue.visible ? setItem.width/2 + 10 : setItem.width/3
				Layout.alignment: Qt.AlignLeft
				showLabel: !btnCopyValue3.visible

				onValueChanged: (str) => {
					tDayModel.setSetWeight(setNumber, exerciseIdx, 0, str);
					text = str;
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue3.visible = true;
				}

				onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();
			}

			RoundButton {
				id: btnCopyValue3
				visible: false
				Layout.alignment: Qt.AlignRight

				Image {
					source: "qrc:/images/"+darkIconFolder+"copy-setvalue.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}

				onClicked: {
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, 0);
					visible = false;
				}
			}

			SetInputField {
				id: txtNWeight2
				type: SetInputField.Type.WeightType
				text: tDayModel.setWeight(setNumber, 1, exerciseIdx);
				availableWidth: setItem.width/4 + 15
				showLabel: false
				Layout.alignment: !btnCopyValue2.visible ? Qt.AlignRight : Qt.AlignLeft

				onValueChanged: (str) => {
					tDayModel.setSetWeight(setNumber, exerciseIdx, 1, str);
					text = str;
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue4.visible = true;
				}

				onEnterOrReturnKeyPressed: {
					const nextSet = itemManager.nextSetObject(exerciseIdx, setNumber);
					if (nextSet)
						nextSet.forceActiveFocus();
				}
			}

			RoundButton {
				id: btnCopyValue4
				visible: false
				Layout.alignment: Qt.AlignRight

				Image {
					source: "qrc:/images/"+darkIconFolder+"copy-setvalue.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}

				onClicked: {
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, 1);
					visible = false;
				}
			}
		}

		SetNotesField {
			id: btnShowHideNotes
		}
	} //ColumnLayout setLayout

	Component.onCompleted: tDayModel.modifiedChanged.connect(hideCopyButtons);

	function changeExerciseText(textObj: var, newtext: string)
	{
		textObj.text = newtext;
	}

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}

	function hideCopyButtons() {
		if (!tDayModel.modified) {
			btnCopyValue.visible = false;
			btnCopyValue2.visible = false;
			btnCopyValue3.visible = false;
			btnCopyValue4.visible = false;
		}
	}

	function changeReps(new_value: string, idx: int) {
		if (idx === 0)
			txtNReps1.text = new_value;
		else
			txtNReps2.text = new_value;
	}

	function changeWeight(new_value: string, idx: int) {
		if (idx === 0)
			txtNWeight1.text = new_value;
		else
			txtNWeight2.text = new_value;
	}
} // Item
