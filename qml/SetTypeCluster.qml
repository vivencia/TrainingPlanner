import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "jsfunctions.js" as JSF

Item {
	id: setItem
	property int setId: -1
	property int exerciseIdx
	property int tDayId
	property int setType: 3 //Constant
	property int setNumber
	property string setReps
	property string setWeight
	property string setSubSets
	property string setRestTime: "00:00"
	property string setNotes: " "
	property var nextObject: null

	signal setRemoved(int nset)
	signal setChanged(int nset, string reps, string weight, string subsets, string resttime, string setnotes)

	implicitHeight: setLayout.implicitHeight
	Layout.fillWidth: true
	Layout.leftMargin: 5

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString() + qsTr("  -  Cluster set")
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
				onClicked: setRemoved(setNumber);
			}

			Label {
				id: lblTotalReps
				text: qsTr("Total reps: ") + parseFloat(setReps) * parseInt(setSubSets)
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
			nSetNbr: setNumber
			text: setNumber !== 0 ? setRestTime : "00:00"
			windowTitle: lblSetNumber.text

			onValueChanged: (str) => {
				setRestTime = str;
				setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
			}

			onEnterOrReturnKeyPressed: {
				txtNSubSets.forceActiveFocus();
			}
		}

		SetInputField {
			id: txtNSubSets
			text: setSubSets
			type: SetInputField.Type.SetType
			nSetNbr: setNumber
			availableWidth: setItem.width

			onEnterOrReturnKeyPressed: {
				txtNReps.forceActiveFocus();
			}

			onValueChanged: (str) => {
				if (str !== setSubSets) {
					setSubSets = str;
					setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
				}
			}
		}

		SetInputField {
			id: txtNReps
			text: setReps.toString()
			type: SetInputField.Type.RepType
			nSetNbr: setNumber
			availableWidth: setItem.width

			onEnterOrReturnKeyPressed: {
				txtNWeight.forceActiveFocus();
			}

			onValueChanged: (str) => {
				if (val !== setReps) {
					setReps = str;
					setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
				}
			}
		}

		SetInputField {
			id: txtNWeight
			text: setWeight.toString()
			type: SetInputField.Type.WeightType
			nSetNbr: setNumber
			availableWidth: setItem.width

			onEnterOrReturnKeyPressed: {
				if (nextObject !== null)
					nextObject.forceActiveFocus()
				else
					txtSetNotes.forceActiveFocus();
			}

			onValueChanged: (str) => {
				if (str !== setWeight) {
					setWeight = str;
					setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
				}
			}
		}

		Label {
			text: qsTr("Notes:")
			Layout.topMargin: 10
			padding: 0
		}
		TextField {
			id: txtSetNotes
			text: setNotes
			font.bold: true
			Layout.fillWidth: true
			Layout.leftMargin: 10
			Layout.rightMargin: 10
			padding: 0

			onTextEdited: {
				if (text.length > 4) {
					setNotes = text;
					setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
				}
			}
		}
	} // setLayout

	function updateTrainingDayId(newTDayId) {
		tDayId = newTDayId;
		setId = -1; //Force a new DB entry to be created when set is logged
	}

	function logSet() {
		if (setNotes === "")
			setNotes = " ";

		if (setId < 0) {
			//console.log("Create Cluster Set# " + setNumber + " - tDayId = " + tDayId + " -1 exerciseIdx = " + exerciseIdx);
			let result = Database.newSetInfo(tDayId, exerciseIdx, setType, setNumber, setReps.toString(),
								setWeight.toString(), AppSettings.weightUnit, setSubSets, setRestTime, setNotes);
			setId = result.insertId;
		}
		else {
			//console.log("Update Cluster Set# " + setNumber + " - tDayId = " + tDayId + " -1 exerciseIdx = " + exerciseIdx);
			Database.updateSetInfo(setId, exerciseIdx, setNumber, setReps.toString(), setWeight.toString(), setSubSets.toString(), setRestTime, setNotes);
		}
	}

	Component.onCompleted: setCreated[setNumber] = 1;
} // FocusScope
