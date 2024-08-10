import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../"
import "../TPWidgets"

import com.vivenciasoftware.qmlcomponents

Item {
	id: setItem
	height: setLayout.height + 15
	implicitHeight: setLayout.implicitHeight + 15
	enabled: tDayModel.dayIsEditable
	Layout.fillWidth: true
	Layout.leftMargin: 5
	Layout.rightMargin: 5

	required property DBTrainingDayModel tDayModel
	required property int exerciseIdx
	required property int setNumber
	required property int setType

	property bool finishButtonVisible: false
	property bool finishButtonEnabled: false
	property bool setCompleted: tDayModel.setCompleted(setNumber, exerciseIdx)
	property var ownerExercise

	signal requestTimerDialogSignal(Item requester, var args)
	signal exerciseCompleted(int exercise_idx)
	signal showRemoveSetMessage(int set_number, int exercise_idx)

	onFocusChanged: {
		if (focus)
			txtNReps1.forceActiveFocus();
	}

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true
		Layout.bottomMargin: 5

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString()
			font.bold: true
			Layout.bottomMargin: 10

			TPComboBox {
				id: cboSetType
				currentIndex: setType
				enabled: !setCompleted
				model: mainwindow.setTypesModel

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

			TPRoundButton {
				id: btnRemoveSet
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: cboSetType.right
				height: 30
				width: 30
				imageName: "remove.png"

				onClicked: showRemoveSetMessage(setNumber, exerciseIdx);
			}

			TPCheckBox {
				id: chkSetCompleted
				text: qsTr("Completed")
				textColor: "black"
				checked: setCompleted
				visible: !setCompleted
				height: 25
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: btnRemoveSet.right

				onCheckedChanged: {
					setCompleted = checked;
					tDayModel.setSetCompleted(setNumber, exerciseIdx, setCompleted);
				}
			}

			Image {
				id: imgCompleted
				source: "qrc:/images/"+darkIconFolder+"set-completed.png"
				asynchronous: true
				fillMode: Image.PreserveAspectFit
				visible: setCompleted
				width: 30
				height: 30
				anchors {
					verticalCenter: parent.verticalCenter
					left: btnRemoveSet.right
					leftMargin: 40
				}

				MouseArea {
					anchors.fill: parent
					onClicked: setCompleted = false;
				}
			}
		}

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			text: tDayModel.setRestTime(setNumber, exerciseIdx);
			availableWidth: setItem.width - 20
			windowTitle: lblSetNumber.text
			visible: setNumber > 0
			enabled: !setCompleted
			Layout.leftMargin: 5

			onValueChanged: (str) => tDayModel.setSetRestTime(setNumber, exerciseIdx, str);

			onEnterOrReturnKeyPressed: txtNReps1.forceActiveFocus();
		}

		RowLayout {
			Layout.fillWidth: true
			enabled: !setCompleted

			ExerciseNameField {
				id: txtExercise1
				objectName: "txtExercise1"
				text: tDayModel.exerciseName1(exerciseIdx)
				showRemoveButton: false
				width: setItem.width/2-10
				Layout.alignment: Qt.AlignLeft
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				onExerciseChanged: (new_text) => {
					tDayModel.setExerciseName1(new_text, exerciseIdx);
					ownerExercise.changeExercise(false);
				}
				onEditButtonClicked: ownerExercise.changeExercise1(!readOnly);
			}

			ExerciseNameField {
				id: txtExercise2
				objectName: "txtExercise2"
				text: tDayModel.exerciseName2(exerciseIdx)
				showRemoveButton: false
				width: setItem.width/2-10
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				onExerciseChanged: (new_text) => {
					tDayModel.setExerciseName2(new_text, exerciseIdx);
					ownerExercise.changeExercise(false);
				}
				onEditButtonClicked: ownerExercise.changeExercise2(!readOnly);
			}
		}

		RowLayout {
			Layout.fillWidth: true
			spacing: 5
			enabled: !setCompleted

			SetInputField {
				id: txtNReps1
				type: SetInputField.Type.RepType
				text: tDayModel.setReps(setNumber, 0, exerciseIdx);
				availableWidth: !btnCopyValue.visible ? setItem.width/2 + 10 : setItem.width/3
				Layout.alignment: Qt.AlignLeft
				showLabel: !btnCopyValue.visible

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, exerciseIdx, 0, str);
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue.visible = true;
				}

				onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();
			}

			TPRoundButton {
				id: btnCopyValue
				visible: false
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, 0);
					visible = false;
				}
			}

			SetInputField {
				id: txtNReps2
				type: SetInputField.Type.RepType
				text: tDayModel.setReps(setNumber, 1, exerciseIdx);
				availableWidth: setItem.width/3
				showLabel: false
				Layout.alignment: !btnCopyValue2.visible ? Qt.AlignRight : Qt.AlignLeft

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, exerciseIdx, 1, str);
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue2.visible = true;
				}

				onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
			}

			TPRoundButton {
				id: btnCopyValue2
				visible: false
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, 1);
					visible = false;
				}
			}
		}

		RowLayout {
			Layout.fillWidth: true
			spacing: 5
			enabled: !setCompleted

			SetInputField {
				id: txtNWeight1
				text: tDayModel.setWeight(setNumber, 0, exerciseIdx);
				type: SetInputField.Type.WeightType
				availableWidth: !btnCopyValue.visible ? setItem.width/2 + 10 : setItem.width/3
				Layout.alignment: Qt.AlignLeft
				showLabel: !btnCopyValue3.visible

				onValueChanged: (str) => {
					tDayModel.setSetWeight(setNumber, exerciseIdx, 0, str);
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue3.visible = true;
				}

				onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();
			}

			TPRoundButton {
				id: btnCopyValue3
				visible: false
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, 0);
					visible = false;
				}
			}

			SetInputField {
				id: txtNWeight2
				type: SetInputField.Type.WeightType
				text: tDayModel.setWeight(setNumber, 1, exerciseIdx);
				availableWidth: setItem.width/3
				showLabel: false
				Layout.alignment: !btnCopyValue2.visible ? Qt.AlignRight : Qt.AlignLeft

				onValueChanged: (str) => {
					tDayModel.setSetWeight(setNumber, exerciseIdx, 1, str);
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue4.visible = true;
				}

				onEnterOrReturnKeyPressed: {
					const nextSet = itemManager.nextSetObject(exerciseIdx, setNumber);
					if (nextSet)
						nextSet.forceActiveFocus();
				}
			}

			TPRoundButton {
				id: btnCopyValue4
				visible: false
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, 1);
					visible = false;
				}
			}
		}

		SetNotesField {
			id: btnShowHideNotes
			text: tDayModel.setNotes(setNumber, exerciseIdx)
			enabled: !setCompleted
			Layout.bottomMargin: 5
			onEditFinished: (new_text) => tDayModel.setSetNotes(setNumber, exerciseIdx, new_text);
		}

		TPButton {
			id: btnCompleteExercise
			text: qsTr("Exercise completed")
			visible: finishButtonVisible
			enabled: finishButtonEnabled
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				setLayout.enabled = false;
				exerciseCompleted(exerciseIdx);
			}
		}
	} //ColumnLayout setLayout

	Component.onCompleted: tDayModel.saveWorkout.connect(hideCopyButtons);

	function changeExerciseText(textObj: var, newtext: string)
	{
		textObj.text = newtext;
	}

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}

	function hideCopyButtons() {
		btnCopyValue.visible = false;
		btnCopyValue2.visible = false;
		btnCopyValue3.visible = false;
		btnCopyValue4.visible = false;
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

	function liberateSignals(liberate: bool) {
		txtExercise1.bCanEmitTextChanged = liberate;
		txtExercise2.bCanEmitTextChanged = liberate;
	}
} // Item
