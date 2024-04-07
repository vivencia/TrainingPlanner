import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

Item {
	id: setItem
	implicitHeight: setLayout.implicitHeight
	Layout.fillWidth: true
	Layout.leftMargin: 5

	required property DBTrainingDayModel tDayModel
	required property int exerciseIdx
	required property int setNumber
	required property string setType

	signal requestTimerDialogSignal(Item requester, var args)

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

			Label {
				id: lblTotalReps
				text: qsTr("Total reps: ") + tDayModel.setReps_int(setNumber, exerciseIdx) * tDayModel.setSubSets_int(setNumber, exerciseIdx)
				height: parent.height
				anchors {
					top: parent.top
					left: btnRemoveSet.right
					leftMargin: 5
				}
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
			onEnterOrReturnKeyPressed: txtNSubSets.forceActiveFocus();
		}

		SetInputField {
			id: txtNSubSets
			type: SetInputField.Type.SetType
			availableWidth: setItem.width

			onValueChanged: (str) => {
				tDayModel.setSetSubSets(setNumber, exerciseIdx, str);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setSubSets(setNumber, exerciseIdx);
			onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
		}

		SetInputField {
			id: txtNReps
			type: SetInputField.Type.RepType
			availableWidth: setItem.width

			onValueChanged: (str) => {
				tDayModel.setSetReps(setNumber, exerciseIdx, str);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setReps(setNumber, exerciseIdx);
			onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
		}

		SetInputField {
			id: txtNWeight
			type: SetInputField.Type.WeightType
			availableWidth: setItem.width

			onValueChanged: (str) => {
				tDayModel.setSetWeight(setNumber, exerciseIdx, str);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setWeight(setNumber, exerciseIdx);
		}

		SetNotesField {
			id: btnShowHideNotes
			Layout.fillWidth: true
		}
	} // setLayout

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}
} // FocusScope
