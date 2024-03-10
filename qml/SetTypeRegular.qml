import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

FocusScope {
	id: setItem
	implicitHeight: setLayout.implicitHeight
	Layout.fillWidth: true
	Layout.leftMargin: 5
	Layout.rightMargin: 5

	required property DBTrainingDayModel tDayModel
	required property int exerciseIdx
	required property int setNumber

	property var nextObject: null
	signal requestTimerDialogSignal(Item requester, var args)

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString() + qsTr("  -  Regular set")
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
				onClicked: {
					appDB.removeSetObject(setNumber, exerciseIdx);
				}
			}
		}

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			availableWidth: setItem.width
			windowTitle: lblSetNumber.text
			focus: setNumber !== 0
			timeSetFirstSet: setNumber === 0;
			timeSetNotFirstSet: setNumber > 0;

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, str, exerciseIdx);
				text = str;
			}

			Component.onCompleted: {
				text = setNumber !== 0 ? tDayModel.setRestTime(setNumber, exerciseIdx) : "00:00";
			}

			onEnterOrReturnKeyPressed: {
				txtNReps.forceActiveFocus();
			}
		}

		SetInputField {
			id: txtNReps
			type: SetInputField.Type.RepType
			availableWidth: setItem.width

			onEnterOrReturnKeyPressed: {
				txtNWeight.forceActiveFocus();
			}

			onValueChanged: (str) => {
				tDayModel.setSetReps(setNumber, str, exerciseIdx);
				text = str;
			}

			Component.onCompleted: {
				text = tDayModel.setReps(setNumber, exerciseIdx);
			}
		}

		SetInputField {
			id: txtNWeight
			type: SetInputField.Type.WeightType
			availableWidth: setItem.width

			onEnterOrReturnKeyPressed: {
				if (nextObject !== null)
					nextObject.forceActiveFocus()
				else
					txtSetNotes.forceActiveFocus();
			}

			onValueChanged: (str) => {
				tDayModel.setSetWeight(setNumber, str, exerciseIdx);
				text = str;
			}

			Component.onCompleted: {
				text = tDayModel.setWeight(setNumber, exerciseIdx);
			}
		}

		Label {
			text: qsTr("Notes:")
			Layout.topMargin: 10
			padding: 0
		}
		TextField {
			id: txtSetNotes
			font.bold: true
			Layout.fillWidth: true
			Layout.leftMargin: 10
			Layout.rightMargin: 10
			padding: 0

			onTextEdited: {
				tDayModel.setSetNotes(text, exerciseIdx);
			}

			Component.onCompleted: {
				text = tDayModel.setNotes(setNumber, exerciseIdx);
			}
		}
	} // setLayout

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}
} // FocusScope
