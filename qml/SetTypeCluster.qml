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
			text: qsTr("Set #") + (setNumber + 1).toString() + "  -  " + mainwindow.setTypesModel[setType].text
			font.bold: true

			RoundButton {
				id: btnRemoveSet
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.right
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
				tDayModel.setSetRestTime(setNumber, str, exerciseIdx);
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
				tDayModel.setSetSubSets(setNumber, str, exerciseIdx);
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
				tDayModel.setSetReps(setNumber, str, exerciseIdx);
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
				tDayModel.setSetWeight(setNumber, str, exerciseIdx);
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
