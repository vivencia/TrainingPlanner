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
	readonly property int setType: 0 //Constant

	property var nextObject: null

	property var myoLabels: [ qsTr("Weight:"), setNumber === 0 ? qsTr("Reps to failure:") : qsTr("Reps to match:"),
						qsTr("Rest time:"), qsTr("Number of short rest pauses:") ]

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString() + qsTr("  -  Myo Reps")
			font.bold: true
			Layout.row: 0
			Layout.column: 0
			Layout.columnSpan: 2

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
			focus: setNumber !== 0
			timeSetFirstSet: setNumber === 0;
			timeSetNotFirstSet: setNumber > 0;
			windowTitle: lblSetNumber.text

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, str, exerciseIdx);
				text = str;
			}

			Component.onCompleted: {
				text = setNumber !== 0 ? tDayModel.setRestTime(setNumber, exerciseIdx) : "00:00";
			}
			onEnterOrReturnKeyPressed: {
				txtNSubSets.forceActiveFocus();
			}
		}

		SetInputField {
			id: txtNSubSets
			type: SetInputField.Type.SetType
			availableWidth: setItem.width
			alternativeLabels: myoLabels

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, str, exerciseIdx);
				text = str;
			}

			onEnterOrReturnKeyPressed: {
				txtNReps.forceActiveFocus();
			}

			Component.onCompleted: {
				text = setNumber !== 0 ? tDayModel.setRestTime(setNumber, exerciseIdx) : "00:00";
			}
		}

		SetInputField {
			id: txtNReps
			type: SetInputField.Type.RepType
			availableWidth: setItem.width
			alternativeLabels: myoLabels

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
} // FocusScope
