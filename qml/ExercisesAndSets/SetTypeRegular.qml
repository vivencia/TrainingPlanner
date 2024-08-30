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
	property bool setCompleted
	property bool bTrackRestTime
	property bool bAutoRestTime
	property int setMode
	readonly property int controlWidth: setItem.width - 20

	readonly property var myoLabels: setType === 5 ? [ qsTr("Weight:"), setNumber === 0 ? qsTr("Reps to failure:") : qsTr("Reps to match:"),
						qsTr("Rest time:"), qsTr("Number of short rest pauses:") ] : []

	signal requestTimerDialogSignal(Item requester, var args)
	signal exerciseCompleted(int exercise_idx)
	signal showRemoveSetMessage(int set_number, int exercise_idx)

	onFocusChanged: {
		if (focus)
			txtNReps.forceActiveFocus();
	}

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true
		Layout.bottomMargin: 5

		Item {
			Layout.fillWidth: true
			height: 30

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
				id: btnCopyValue3
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
				height: 20
				width: 20

				anchors {
					verticalCenter: parent.verticalCenter
					left: btnCopyValue3.visible ? btnCopyValue3.right : cboSetType.right
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

				onEnterOrReturnKeyPressed: {
					if (txtNSubSets.visible)
						txtNSubSets.forceActiveFocus();
					else
						txtNReps.forceActiveFocus();
				}
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

		SetInputField {
			id: txtNSubSets
			type: SetInputField.Type.SetType
			text: tDayModel.setSubSets(setNumber, exerciseIdx);
			availableWidth: controlWidth
			visible: setType === 3 || setType === 5
			alternativeLabels: myoLabels
			enabled: !setCompleted
			Layout.leftMargin: 5

			onValueChanged: (str) => {
				tDayModel.setSetSubSets(setNumber, exerciseIdx, str);
				if (setType === 3)
					changeTotalRepsLabel();
			}

			onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();

			Label {
				id: lblTotalReps
				font.pointSize: AppSettings.fontSizeText
				font.bold: true
				color: AppSettings.fontColor
				visible: setType === 3				
				anchors {
					top: parent.verticalCenter
					topMargin: -height/2
					left: parent.horizontalCenter
					leftMargin: 10
				}
			}
		}

		RowLayout {
			enabled: !setCompleted

			SetInputField {
				id: txtNReps
				type: SetInputField.Type.RepType
				text: tDayModel.setReps(setNumber, exerciseIdx);
				availableWidth: copyRepsButtonValue === "" ? controlWidth : controlWidth - 40
				alternativeLabels: myoLabels
				Layout.leftMargin: 5

				onValueChanged: (str) => {
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
						if (copyRepsButtonValue === str)
							copyRepsButtonValue = "";
						else if (copyRepsButtonValue === "")
							copyRepsButtonValue = tDayModel.setReps(setNumber, exerciseIdx);
					}
					tDayModel.setSetReps(setNumber, exerciseIdx, str);
					if (setType === 3)
						changeTotalRepsLabel();
				}

				onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
			}

			TPButton {
				id: btnCopyValue
				visible: copyRepsButtonValue !== ""
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber);
					copyRepsButtonValue = "";
				}
			}
		} //RowLayout

		RowLayout {
			Layout.leftMargin: 5
			enabled: !setCompleted

			SetInputField {
				id: txtNWeight
				type: SetInputField.Type.WeightType
				text: tDayModel.setWeight(setNumber, exerciseIdx);
				availableWidth: copyWeightButtonValue === "" ? controlWidth : controlWidth - 40
				alternativeLabels: myoLabels

				onValueChanged: (str) => {
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
						if (copyWeightButtonValue === str)
							copyWeightButtonValue = "";
						else if (copyWeightButtonValue === "")
							copyWeightButtonValue = tDayModel.setWeight(setNumber, exerciseIdx);
					}
					tDayModel.setSetWeight(setNumber, exerciseIdx, str);
				}

				onEnterOrReturnKeyPressed: {
					const nextSet = itemManager.nextSetObject(exerciseIdx, setNumber);
					if (nextSet)
						nextSet.forceActiveFocus();
				}
			}

			TPButton {
				id: btnCopyValue2
				visible: copyWeightButtonValue !== ""
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber);
					copyWeightButtonValue = "";
				}
			}
		} //RowLayout

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
			Layout.alignment: Qt.AlignHCenter

			onClicked: {
				setLayout.enabled = false;
				exerciseCompleted(exerciseIdx);
			}
		}
	} // setLayout

	Component.onCompleted: {
		tDayModel.saveWorkout.connect(hideCopyButtons);
		if (setType === 3)
			changeTotalRepsLabel();
	}

	function changeTotalRepsLabel() {
		lblTotalReps.text = qsTr("Total reps: ") + tDayModel.setReps_int(setNumber, exerciseIdx) * tDayModel.setSubSets_int(setNumber, exerciseIdx);
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
		txtNReps.text = new_value;
		if (setType === 3)
			changeTotalRepsLabel();
	}

	function changeWeight(new_value: string, idx: int) {
		txtNWeight.text = new_value;
	}

	function updateRestTime(str_time: string) {
		txtRestTime.text = str_time;
	}
} // Item
