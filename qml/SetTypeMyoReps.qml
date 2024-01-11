import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "jsfunctions.js" as JSF

Item {
	id: setItem
	property int setId
	property int exerciseId
	property int tDayId
	property int setType: 5 //Constant
	property int setNumber
	property real setReps
	property real setWeight
	property string setWeightUnit
	property int setSubSets: 0
	property string setRestTime: "00:00"
	property string setNotes: " "

	property var myoLabels: [ qsTr("Weight:"), setNumber === 0 ? qsTr("Reps to failure:") : qsTr("Reps to match:"),
						qsTr("Rest time:"), qsTr("Number of short rest pauses:") ]

	property bool bIsRemoving: false

	signal setRemoved(int nset)
	signal setChanged(int nset, int reps, int weight, int subsets, string resttime, string setnotes)

	implicitHeight: setLayout.implicitHeight
	Layout.fillWidth: true
	Layout.leftMargin: 5

	onSetNumberChanged: { //This is changed in ExerciseEntry.qml when a set is removed
		if (bIsRemoving) {
			bIsRemoving = false;
			if (setId > 0) {
				Database.deleteSetFromSetsInfo(setId);
			}
		}
	}

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #: ") + (setNumber + 1).toString() + qsTr("  -  Myo Reps")
			font.bold: true
			Layout.row: 0
			Layout.column: 0
			Layout.columnSpan: 2

			ToolButton {
				id: btnRemoveSet
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.right
				height: 20
				width: 20
				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 25
					width: 25
				}
				onClicked: setRemoved(setNumber);
			}
		}

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			availableWidth: setItem.width
			nSetNbr: setNumber
			text: setNumber !== 0 ? setRestTime : "00:00"
			windowTitle: lblSetNumber.text

			onValueChanged: (str, val) => {
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
			alternativeLabels: myoLabels

			onEnterOrReturnKeyPressed: {
				txtNReps.forceActiveFocus();
			}

			onValueChanged: (str, val) => {
				if (val !== setSubSets) {
					setSubSets = val;
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
			alternativeLabels: myoLabels

			onEnterOrReturnKeyPressed: {
				txtNWeight.forceActiveFocus();
			}

			onValueChanged: (str, val) => {
				if (val !== setReps) {
					setReps = val;
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
				txtSetNotes.forceActiveFocus();
			}

			onValueChanged: (str, val) => {
				if (val !== setWeight) {
					setWeight = val;
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
			console.log("SetTypeRegular: #" + setNumber.toString() + "  " + exerciseId.toString() + "   " + tDayId.toString());
			let result = Database.newSetInfo(tDayId, exerciseId, setType, setNumber, setReps.toString(),
								setWeight.toString(), AppSettings.weightUnit, setSubSets, setRestTime, setNotes);
			setId = result.insertId;
		}
		else {
			Database.updateSetInfo(setId, exerciseId, setNumber, setReps.toString(), setWeight.toString(), setSubSets.toString(), setRestTime, setNotes);
			/*Database.updateSetInfo_setNumber(setId, setNumber);
			Database.updateSetInfo_setReps(setId, setReps);
			Database.updateSetInfo_setWeight(setId, setWeight);
			Database.updateSetInfo_setNotes(setId, setNotes);
			Database.updateSetInfo_setRestTime(setId, setRestTime);
			Database.updateSetInfo_setSubSets(setId, setSubSets);*/
		}
	}
} // Item
