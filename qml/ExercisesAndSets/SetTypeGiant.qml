import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

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

	property var ownerExercise
	property string copyTypeButtonValue: ""
	property string copyTimeButtonValue: ""
	property string copyRepsButtonValue: ""
	property string copyWeightButtonValue: ""
	property bool finishButtonVisible: false
	property bool finishButtonEnabled: false
	property bool setCompleted
	property bool bTrackRestTime
	property bool bAutoRestTime
	property bool bCurrentSet
	property int setMode

	readonly property int controlWidth: setItem.width - 20

	signal requestTimerDialogSignal(Item requester, var args)
	signal exerciseCompleted(int exercise_idx)
	signal showRemoveSetMessage(int set_number, int exercise_idx)

	onFocusChanged: {
		if (focus)
			txtNReps1.forceActiveFocus();
	}

	Rectangle {
		id: indicatorRec
		visible: false
		color: AppSettings.entrySelectedColor
		layer.enabled: true
		radius: 5
		border.color: "#707d8d"
		border.width: 1
		anchors.fill: parent
	}

	MultiEffect {
		id: currentSetEffect
		visible: bCurrentSet
		source: indicatorRec
		shadowEnabled: true
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1
		opacity: 0.5
		anchors.fill: parent
	}

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true
		Layout.bottomMargin: 5

		Item {
			height: 30
			Layout.fillWidth: true
			Layout.topMargin: 10

			TPButton {
				id: btnManageSet
				text: setMode === 0 ? qsTr("Set Completed") : (setMode === 1 ? qsTr("Start Rest") : qsTr("Start Exercise"))
				flat: false
				visible: !setCompleted
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked: itemManager.changeSetMode(exerciseIdx, setNumber);
			}

			TPButton {
				id: imgCompleted
				imageSource: "set-completed"
				visible: setCompleted
				height: 30
				width: 30
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked: itemManager.changeSetMode(exerciseIdx, setNumber);
			}
		}

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString()
			font.bold: true
			Layout.topMargin: 10
			Layout.bottomMargin: 10

			TPComboBox {
				id: cboSetType
				currentIndex: setType
				enabled: !setCompleted
				model: AppSettings.setTypesModel
				implicitWidth: 160

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

			TPButton {
				id: btnCopyValue5
				visible: copyTypeButtonValue !== ""
				imageSource: "copy-setvalue"
				height: 25
				width: 25

				anchors {
					verticalCenter: parent.verticalCenter
					left: cboSetType.right
					leftMargin: 10
				}

				onClicked: {
					itemManager.copyTypeValueIntoOtherSets(exerciseIdx, setNumber);
					copyTypeButtonValue = "";
				}
			}

			TPButton {
				id: btnRemoveSet
				imageSource: "remove"
				height: 25
				width: 25

				anchors {
					verticalCenter: parent.verticalCenter
					left: btnCopyValue5.visible ? btnCopyValue5.right : cboSetType.right
					leftMargin: 10
				}

				onClicked: showRemoveSetMessage(setNumber, exerciseIdx);
			}
		}

		RowLayout {
			visible: setNumber > 0 && bTrackRestTime
			enabled: !setCompleted
			Layout.leftMargin: 5

			SetInputField {
				id: txtRestTime
				type: SetInputField.Type.TimeType
				text: tDayModel.setRestTime(setNumber, exerciseIdx);
				availableWidth: copyTimeButtonValue === "" ? controlWidth : controlWidth - 40
				windowTitle: lblSetNumber.text
				showButtons: !bAutoRestTime

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

			TPButton {
				id: btnCopyTimeValue
				visible: copyTimeButtonValue !== ""
				imageSource: "copy-setvalue"
				height: 25
				width: 25
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
				width: controlWidth/2 + 10
				Layout.maximumWidth: width
				Layout.minimumWidth: width
				Layout.leftMargin: -10

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
				width: controlWidth/2 + 10
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: width
				Layout.minimumWidth: width
				Layout.leftMargin: -10

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
				availableWidth: copyRepsButtonValue === "" ? controlWidth/2 + 10 : controlWidth/3
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

			TPButton {
				id: btnCopyValue
				visible: copyRepsButtonValue !== ""
				imageSource: "copy-setvalue"
				height: 25
				width: 25
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
				availableWidth: controlWidth/3
				showLabel: false
				//Layout.alignment: copyRepsButtonValue === "" ? Qt.AlignRight : Qt.AlignLeft

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

			TPButton {
				id: btnCopyValue2
				visible: copyRepsButtonValue !== ""
				imageSource: "copy-setvalue"
				height: 25
				width: 25
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
				availableWidth: copyWeightButtonValue === "" ? controlWidth/2 + 10 : controlWidth/3
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

			TPButton {
				id: btnCopyValue3
				visible: copyWeightButtonValue !== ""
				imageSource: "copy-setvalue"
				height: 25
				width: 25
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
				availableWidth: controlWidth/3 + 10
				showLabel: false
				//Layout.alignment: copyWeightButtonValue === "" ? Qt.AlignRight : Qt.AlignLeft

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

			TPButton {
				id: btnCopyValue4
				visible: copyWeightButtonValue !== ""
				imageSource: "copy-setvalue"
				height: 25
				width: 25
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
			Layout.leftMargin: 5
			Layout.rightMargin: 5
			Layout.fillWidth: true

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

	function updateRestTime(str_time: string) {
		txtRestTime.text = str_time;
	}
} // Item
