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

	signal requestTimerDialogSignal(Item requester, var args)

	readonly property int controlWidth: setItem.width - 20

	readonly property var myoLabels: setType === 5 ? [ qsTr("Weight:"), setNumber === 0 ? qsTr("Reps to failure:") : qsTr("Reps to match:"),
						qsTr("Rest time:"), qsTr("Number of short rest pauses:") ] : []

	onFocusChanged: {
		if (focus)
			txtNReps.forceActiveFocus();
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
				height: 30
				width: 30

				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 30
					width: 30
				}
				onClicked: itemManager.removeSetObject(setNumber, exerciseIdx);
			}
		}

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			text: tDayModel.setRestTime(setNumber, exerciseIdx);
			availableWidth: controlWidth
			windowTitle: lblSetNumber.text
			visible: setNumber > 0
			Layout.leftMargin: 5

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, exerciseIdx, str);
				text = str;
			}

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
			Layout.leftMargin: 5

			onValueChanged: (str) => {
				tDayModel.setSetSubSets(setNumber, exerciseIdx, str);
				text = str;
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
			SetInputField {
				id: txtNReps
				type: SetInputField.Type.RepType
				text: tDayModel.setReps(setNumber, exerciseIdx);
				availableWidth: !btnCopyValue.visible ? controlWidth : controlWidth - 40
				alternativeLabels: myoLabels
				Layout.leftMargin: 5

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, exerciseIdx, str);
					text = str;
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue.visible = true;
					if (setType === 3)
						changeTotalRepsLabel();
				}

				onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
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
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber);
					btnCopyValue.visible = false;
				}
			}
		} //RowLayout

		RowLayout {
			Layout.leftMargin: 5

			SetInputField {
				id: txtNWeight
				type: SetInputField.Type.WeightType
				text: tDayModel.setWeight(setNumber, exerciseIdx);
				availableWidth: !btnCopyValue2.visible ? controlWidth : controlWidth - 40
				alternativeLabels: myoLabels

				onValueChanged: (str) => {
					tDayModel.setSetWeight(setNumber, exerciseIdx, str);
					text = str;
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue2.visible = true;
				}

				onEnterOrReturnKeyPressed: {
					const nextSet = itemManager.nextSetObject(exerciseIdx, setNumber);
					if (nextSet)
						nextSet.forceActiveFocus();
				}
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
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber);
					btnCopyValue2.visible = false;
				}
			}
		} //RowLayout

		SetNotesField {
			id: btnShowHideNotes
			text: tDayModel.setNotes(setNumber, exerciseIdx)
			onEditFinished: (new_text) => tDayModel.setSetNotes(setNumber, exerciseIdx, new_text);
		}
	} // setLayout

	Component.onCompleted: {
		tDayModel.modifiedChanged.connect(hideCopyButtons);
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
		if (!tDayModel.modified) {
			btnCopyValue.visible = false;
			btnCopyValue2.visible = false;
		}
	}

	function changeReps(new_value: string, idx: int) {
		txtNReps.text = new_value;
		if (setType === 3)
			changeTotalRepsLabel();
	}

	function changeWeight(new_value: string, idx: int) {
		txtNWeight.text = new_value;
	}
} // Item
