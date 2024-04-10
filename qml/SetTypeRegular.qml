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

	signal requestTimerDialogSignal(Item requester, var args)

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
			onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
		}

		RowLayout {
			SetInputField {
				id: txtNReps
				type: SetInputField.Type.RepType
				availableWidth: !btnCopyValue.visible ? setItem.width : setItem.width - 60

				onValueChanged: (str) => {
					tDayModel.setSetReps(setNumber, exerciseIdx, str);
					text = str;
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
						btnCopyValue.visible = true;
				}

				Component.onCompleted: text = tDayModel.setReps(setNumber, exerciseIdx);
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
					itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, txtNReps.text);
					btnCopyValue.visible = false;
				}
			}
		}

		RowLayout {
			SetInputField {
				id: txtNWeight
				type: SetInputField.Type.WeightType
				availableWidth: !btnCopyValue2.visible ? setItem.width : setItem.width - 60

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

				Component.onCompleted: text = tDayModel.setWeight(setNumber, exerciseIdx);
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
					itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, txtNWeight.text);
					btnCopyValue2.visible = false;
				}
			}
		}

		SetNotesField {
			id: btnShowHideNotes
		}
	} // setLayout

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}

	function changeReps(new_value: string) {
		txtNReps.text = new_value;
	}

	function changeWeight(new_value: string) {
		txtNWeight.text = new_value;
	}
} // Item
