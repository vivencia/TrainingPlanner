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
	property string copyTypeButtonValue: ""
	property string copyTimeButtonValue: ""
	property string copyRepsButtonValue: ""
	property string copyWeightButtonValue: ""
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
				model: AppSettings.setTypesModel

				anchors {
					left: parent.right
					leftMargin: 10
					verticalCenter: parent.verticalCenter
				}

				onActivated: (index)=> {
					if (index !== setType) {
						if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
							if (copyTypeButtonValue === cboSetType.textAt(index))
								copyTypeButtonValue = "";
							else if (copyTypeButtonValue === "")
								copyTypeButtonValue = tDayModel.setType(setNumber, exerciseIdx).toString();
						}
						itemManager.changeSetType(setNumber, exerciseIdx, index);
					}
				}
			}

			TPRoundButton {
				id: btnCopyValue5
				visible: copyTypeButtonValue !== ""
				imageName: "copy-setvalue.png"
				width: 25
				height: 25

				anchors {
					verticalCenter: parent.verticalCenter
					left: cboSetType.right
				}

				onClicked: {
					itemManager.copyTypeValueIntoOtherSets(exerciseIdx, setNumber);
					copyTypeButtonValue = "";
				}
			}

			TPRoundButton {
				id: btnRemoveSet
				imageName: "remove.png"
				height: 30
				width: 30

				anchors {
					verticalCenter: parent.verticalCenter
					left: btnCopyValue5.visible ? btnCopyValue5.right : cboSetType.right
				}

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
				source: "qrc:/images/set-completed.png"
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

		RowLayout {
			visible: setNumber > 0
			enabled: !setCompleted
			Layout.leftMargin: 5

			SetInputField {
				id: txtRestTime
				type: SetInputField.Type.TimeType
				text: tDayModel.setRestTime(setNumber, exerciseIdx);
				availableWidth: copyTimeButtonValue === "" ? controlWidth : controlWidth - 40
				windowTitle: lblSetNumber.text

				onValueChanged: (str) => {
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
						if (copyTimeButtonValue === str)
							copyTimeButtonValue = "";
						else if (copyTimeButtonValue === "")
							copyTimeButtonValue = tDayModel.setRestTime(setNumber, exerciseIdx);
					}
					tDayModel.setSetRestTime(setNumber, exerciseIdx, str);
				}

				onEnterOrReturnKeyPressed: txtNReps1.forceActiveFocus();
			}

			TPRoundButton {
				id: btnCopyTimeValue
				visible: copyTimeButtonValue !== ""
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyTimeValueIntoOtherSets(exerciseIdx, setNumber);
					copyTimeButtonValue = "";
				}
			}
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
				availableWidth: copyRepsButtonValue === "" ? setItem.width/2 + 10 : setItem.width/3
				Layout.alignment: Qt.AlignLeft
				showLabel: !copyRepsButtonValue

				onValueChanged: (str) => {
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
						if (copyRepsButtonValue === str)
							copyRepsButtonValue = "";
						else if (copyRepsButtonValue === "")
							copyRepsButtonValue = tDayModel.setReps(setNumber, 0, exerciseIdx);
					}
					tDayModel.setSetReps(setNumber, exerciseIdx, 0, str);
				}

				onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();
			}

			TPRoundButton {
				id: btnCopyValue
				visible: copyRepsButtonValue !== ""
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, 0);
					copyRepsButtonValue = "";
				}
			}

			SetInputField {
				id: txtNReps2
				type: SetInputField.Type.RepType
				text: tDayModel.setReps(setNumber, 1, exerciseIdx);
				availableWidth: setItem.width/3
				showLabel: false
				Layout.alignment: copyRepsButtonValue === "" ? Qt.AlignRight : Qt.AlignLeft

				onValueChanged: (str) => {
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
						if (copyRepsButtonValue === str)
							copyRepsButtonValue = "";
						else if (copyRepsButtonValue === "")
							copyRepsButtonValue = tDayModel.setReps(setNumber, 1, exerciseIdx);
					}
					tDayModel.setSetReps(setNumber, exerciseIdx, 1, str);
				}

				onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
			}

			TPRoundButton {
				id: btnCopyValue2
				visible: copyRepsButtonValue !== ""
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, 1);
					copyRepsButtonValue = "";
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
				availableWidth: copyWeightButtonValue === "" ? setItem.width/2 + 10 : setItem.width/3
				Layout.alignment: Qt.AlignLeft
				showLabel: !copyWeightButtonValue

				onValueChanged: (str) => {
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
						if (copyWeightButtonValue === str)
							copyWeightButtonValue = "";
						else if (copyWeightButtonValue === "")
							copyWeightButtonValue = tDayModel.setWeight(setNumber, 0, exerciseIdx);
					}
					tDayModel.setSetWeight(setNumber, exerciseIdx, 0, str);
				}

				onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();
			}

			TPRoundButton {
				id: btnCopyValue3
				visible: copyWeightButtonValue !== ""
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, 0);
					copyWeightButtonValue = "";
				}
			}

			SetInputField {
				id: txtNWeight2
				type: SetInputField.Type.WeightType
				text: tDayModel.setWeight(setNumber, 1, exerciseIdx);
				availableWidth: setItem.width/3
				showLabel: false
				Layout.alignment: copyWeightButtonValue === "" ? Qt.AlignRight : Qt.AlignLeft

				onValueChanged: (str) => {
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
						if (copyWeightButtonValue === str)
							copyWeightButtonValue = "";
						else if (copyWeightButtonValue === "")
							copyWeightButtonValue = tDayModel.setWeight(setNumber, 1, exerciseIdx);
					}
					tDayModel.setSetWeight(setNumber, exerciseIdx, 1, str);
				}

				onEnterOrReturnKeyPressed: {
					const nextSet = itemManager.nextSetObject(exerciseIdx, setNumber);
					if (nextSet)
						nextSet.forceActiveFocus();
				}
			}

			TPRoundButton {
				id: btnCopyValue4
				visible: copyWeightButtonValue !== ""
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, 1);
					copyWeightButtonValue = "";
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
		copyTypeButtonValue = "";
		copyRepsButtonValue = "";
		copyWeightButtonValue = "";
	}

	function changeSetType(new_type: int) {
		cboSetType.currentIndex = new_type;
	}

	function changeTime(new_time: string) {
		txtRestTime.text = new_time;
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
