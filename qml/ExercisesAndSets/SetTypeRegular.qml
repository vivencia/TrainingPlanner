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
	property bool copyTypeButtonVisible: false
	property bool copyRepsButtonVisible: false
	property bool copyWeightButtonVisible: false
	property bool setCompleted: tDayModel.setCompleted(setNumber, exerciseIdx)
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
					if (index !== setType) {
						if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
							copyTypeButtonVisible = true;
						itemManager.changeSetType(setNumber, exerciseIdx, index);
					}
				}
			}

			TPRoundButton {
				id: btnCopyValue3
				visible: copyTypeButtonVisible
				imageName: "copy-setvalue.png"
				width: 25
				height: 25

				anchors {
					verticalCenter: parent.verticalCenter
					left: cboSetType.right
				}

				onClicked: {
					itemManager.copyTypeValueIntoOtherSets(exerciseIdx, setNumber);
					copyTypeButtonVisible = false;
				}
			}

			TPRoundButton {
				id: btnRemoveSet
				height: 30
				width: 30
				imageName: "remove.png"

				anchors {
					verticalCenter: parent.verticalCenter
					left: btnCopyValue3.visible ? btnCopyValue3.right : cboSetType.right
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
					onClicked: {
						setCompleted = false;
						tDayModel.setSetCompleted(setNumber, exerciseIdx, setCompleted);
					}
				}
			}
		}

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			text: tDayModel.setRestTime(setNumber, exerciseIdx);
			availableWidth: controlWidth
			windowTitle: lblSetNumber.text
			visible: setNumber > 0
			enabled: !setCompleted
			Layout.leftMargin: 5

			onValueChanged: (str) => tDayModel.setSetRestTime(setNumber, exerciseIdx, str);

			onEnterOrReturnKeyPressed: {
				if (txtNSubSets.visible)
					txtNSubSets.forceActiveFocus();
				else
					txtNReps.forceActiveFocus();
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
				availableWidth: !copyRepsButtonVisible ? controlWidth : controlWidth - 40
				alternativeLabels: myoLabels
				Layout.leftMargin: 5

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, exerciseIdx, str);
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						copyRepsButtonVisible = true;
					if (setType === 3)
						changeTotalRepsLabel();
				}

				onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
			}

			TPRoundButton {
				id: btnCopyValue
				visible: copyRepsButtonVisible
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber);
					copyRepsButtonVisible = false;
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
				availableWidth: !copyWeightButtonVisible ? controlWidth : controlWidth - 40
				alternativeLabels: myoLabels

				onValueChanged: (str) => {
					tDayModel.setSetWeight(setNumber, exerciseIdx, str);
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						copyWeightButtonVisible = true;
				}

				onEnterOrReturnKeyPressed: {
					const nextSet = itemManager.nextSetObject(exerciseIdx, setNumber);
					if (nextSet)
						nextSet.forceActiveFocus();
				}
			}

			TPRoundButton {
				id: btnCopyValue2
				visible: copyWeightButtonVisible
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber);
					copyWeightButtonVisible = false;
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
		copyTypeButtonVisible = false;
		copyRepsButtonVisible = false;
		copyWeightButtonVisible = false;
	}

	function changeReps(new_value: string, idx: int) {
		txtNReps.text = new_value;
		if (setType === 3)
			changeTotalRepsLabel();
	}

	function changeWeight(new_value: string, idx: int) {
		txtNWeight.text = new_value;
	}

	function changeSetType(new_type: int) {
		cboSetType.currentIndex = new_type;
	}
} // Item
