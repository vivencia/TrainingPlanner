import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "jsfunctions.js" as JSF

Item {
	id: setItem
	property int setId
	property int exerciseId
	property int tDayId
	property int setType: 4 //Constant
	property int setNumber
	property string setReps
	property string setWeight
	property string setWeightUnit
	property int setSubSets: 0
	property string setRestTime: "00:00"
	property string setNotes: " "

	property string exerciseName2
	property string subName2
	property bool bIsRemoving: false
	property bool bIgnoreExerciseIdChange: false
	property bool bUpdateLists
	property var subSetList: []
	property var stackViewObj

	signal setRemoved(int nset)
	signal setChanged(int nset, string reps, string weight, int subsets, string resttime, string setnotes)
	signal exerciseIdsChanged(int old_exerciseId, int new_exerciseid)

	property int exerciseId2: -1
	property int exerciseId1: -1
	property string strReps1
	property string strReps2
	property string strWeight1
	property string strWeight2

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

	onExerciseIdChanged: {
		if (!bIgnoreExerciseIdChange) {
			exerciseId1 = JSF.getExerciseIdFromCompositeExerciseId(0, exerciseId);
			exerciseId2 = JSF.getExerciseIdFromCompositeExerciseId(1, exerciseId);
			bIgnoreExerciseIdChange = true;
			exerciseId = JSF.toCompositeExerciseId(exerciseId1, exerciseId2);
			bIgnoreExerciseIdChange = false;
		}
	}

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #: ") + (setNumber + 1).toString() + qsTr("  -  Giant set")
			font.bold: true

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

		TextInput {
			id: txtExerciseName2
			text: exerciseName2
			font.bold: true
			readOnly: true
			wrapMode: Text.WordWrap
			Layout.topMargin: 2
			Layout.leftMargin: 5
			Layout.fillWidth: true
			z: 1

			ToolButton {
				id: btnRemoveExercise2
				padding: 10
				anchors.right: parent.right
				anchors.top: txtExerciseName2.top
				anchors.rightMargin: 20
				height: 25
				width: 25
				visible: setNumber === 0

				z: 2
				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.fill: parent
				}

				onClicked: {
					removeSecondExercise();
				}
			} //btnRemoveExercise2

			ToolButton {
				id: btnEditExercise2
				padding: 10
				anchors.right: btnRemoveExercise2.left
				anchors.top: txtExerciseName2.top
				height: 25
				width: 25
				z: 2
				visible: setNumber === 0

				Image {
					source: "qrc:/images/"+darkIconFolder+"edit.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}

				onClicked: {
					const exerciseid = [exerciseId1, exerciseId2];
					var exercise = stackViewObj.push("ExercisesDatabase.qml", { bChooseButtonEnabled: true, doNotChooseTheseIds:exerciseid });
						exercise.exerciseChosen.connect(exerciseReceived);
				}
			} //btnEditExercise2
		} //txtExerciseName2

		TextInput {
			id: txtSubName2
			text: subName2
			font.italic: true
			readOnly: true
			wrapMode: Text.WordWrap
			Layout.fillWidth: false
			Layout.maximumWidth: exerciseItem.width - 20
			visible: setNumber === 0
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
		}

		Rectangle {
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignCenter
			Layout.topMargin: 10
			Layout.bottomMargin: 10

			Label {
				id: lblExercise1
				text: qsTr("Exercise 1")
				width: setItem.width/2
				font.bold: true

				anchors {
					left: parent.left
					leftMargin: width/2 - implicitWidth/2
					verticalCenter: parent.verticalCenter
				}
			}
			Label {
				id: lblExercise2
				text: qsTr("Exercise 2")
				width: setItem.width/2
				font.bold: true

				anchors {
					left: parent.horizontalCenter
					leftMargin: width/2 - implicitWidth/2
					verticalCenter: parent.verticalCenter
				}
			}
		}

		Rectangle {
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignCenter
			Layout.topMargin: 10
			Layout.bottomMargin: 10

			Label {
				text: qsTr("Reps/Weight") + AppSettings.weightUnit
				width: setItem.width/2

				anchors {
					left: parent.left
					leftMargin: width/2 - implicitWidth/2
					verticalCenter: parent.verticalCenter
				}
			}
			Label {
				text: qsTr("Reps/Weight") + AppSettings.weightUnit
				width: setItem.width/2

				anchors {
					left: parent.horizontalCenter
					leftMargin: width/2 - implicitWidth/2
					verticalCenter: parent.verticalCenter
				}
			}
		}

		RowLayout {
			id: fldsLayout
			Layout.fillWidth: true
			spacing: 0

			SetInputField {
				id: txtNReps1
				text: strReps1
				type: SetInputField.Type.RepType
				nSetNbr: setNumber
				availableWidth: setItem.width/4
				alternativeLabels: ["","","",""]

				onEnterOrReturnKeyPressed: {
					txtNWeight1.forceActiveFocus();
				}

				onValueChanged: (str, val) => {
					onTextEdited: changeRep(0, str);
				}
			}
			SetInputField {
				id: txtNWeight1
				text: strWeight1
				type: SetInputField.Type.WeightType
				nSetNbr: setNumber
				availableWidth: setItem.width/4
				alternativeLabels: ["","","",""]

				onEnterOrReturnKeyPressed: {
					txtNReps2.forceActiveFocus();
				}

				onValueChanged: (str, val) => {
					changeWeight(0, str);
				}
			}
			SetInputField {
				id: txtNReps2
				text: strReps2
				type: SetInputField.Type.RepType
				nSetNbr: setNumber
				availableWidth: setItem.width/4
				alternativeLabels: ["","","",""]

				onEnterOrReturnKeyPressed: {
					txtNWeight1.forceActiveFocus();
				}

				onValueChanged: (str, val) => {
					onTextEdited: changeRep(1, str);
				}
			}
			SetInputField {
				id: txtNWeight2
				text: strWeight2
				type: SetInputField.Type.WeightType
				nSetNbr: setNumber
				availableWidth: setItem.width/4
				alternativeLabels: ["","","",""]

				onEnterOrReturnKeyPressed: {
					txtSetNotes.forceActiveFocus();
				}

				onValueChanged: (str, val) => {
					changeWeight(1, str);
				}
			}
		} //RowLayout

		Label {
			text: qsTr("Notes:")
			Layout.topMargin: 10
			padding: 0
		}
		TextField {
			id: txtSetNotes
			text: setNotes
			font.bold: true
			readOnly: false
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

		Component.onCompleted:
			getVariables(exerciseId);

	} //ColumnLayout setLayout

	function getVariables(exerciseid) {
		exerciseId = exerciseid;
		exerciseId1 = JSF.getExerciseIdFromCompositeExerciseId(0, exerciseId);
		exerciseId2 = JSF.getExerciseIdFromCompositeExerciseId(1, exerciseId);
		let result = Database.getExercise(exerciseId2);
		if (result.length > 0) {
			exerciseName2 = "2 - " + result[0].exercisePName;
			subName2 = "2 - " + result[0].exerciseSName;
		}
		else {
			exerciseName2 = qsTr("Add exercise");
			subName2 = "";
		}

		var idx = setReps.indexOf(',')
		if (idx === -1) {
			setReps = "10,10"; //Random default values
			idx = 2;
		}
		strReps1 = setReps.substring(0, idx);
		strReps2 = setReps.substring(idx+1, setReps.length);

		idx = setWeight.indexOf(',')
		if (idx === -1) {
			setWeight = "50,50"; //Random default values
			idx = 2;
		}
		strWeight1 = setWeight.substring(0, idx);
		strWeight2 = setWeight.substring(idx+1, setReps.length);
	}

	function changeRep(idx, newRep) {
		const reps = setReps.split(',');
		if (reps[idx] !== newRep) {
			if (idx === 0)
				strReps1 = newRep
			else
				strReps2 = newRep
			setReps = strReps1 + "," + strReps2;
			setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
		}
	}

	function changeWeight(idx, newRep) {
		const weights = setWeight.split(',');
		if (weights[idx] !== newRep) {
			if (idx === 0)
				strWeight1 = newRep;
			else
				strWeight2 = newRep;
			setWeight = strWeight1 + ',' + strWeight2;
			setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
		}
	}

	function exerciseReceived(strName1, strName2, sets, reps, weight, uweight, exerciseid, bAdd) {
		const oldexerciseid = exerciseId;
		exerciseId = JSF.toCompositeExerciseId(exerciseId1, exerciseid);
		exerciseId2 = exerciseid * 10000;
		exerciseIdsChanged(oldexerciseid, exerciseId);
		exerciseName2 = "2 - " + strName1;
		subName2 = "2 - " + strName2;
	}

	function removeSecondExercise() {
		exerciseName2 = qsTr("2 - Add exercise");
		subName2 = qsTr("2 - Add exercise");
		exerciseId2 = -1;
		exerciseIdsChanged(exerciseId, exerciseId1);
		exerciseId = exerciseId1;
	}

	function updateTrainingDayId(newTDayId) {
		tDayId = newTDayId;
		setId = -1; //Force a new DB entry to be created when set is logged
	}

	function logSet() {
		if (setNotes === "")
			setNotes = " ";

		if (setId < 0) {
			console.log("SetGiantDropSet: #" + setNumber.toString() + "  " + exerciseId.toString() + "   " + tDayId.toString());
			let result = Database.newSetInfo(tDayId, exerciseId, setType, setNumber, setReps,
								setWeight, AppSettings.weightUnit, setSubSets, setRestTime, setNotes);
			setId = result.insertId;
		}
		else {
			Database.updateSetInfo(setId, exerciseId, setNumber, setReps, setWeight, setSubSets.toString(), setRestTime, setNotes);
			/*Database.updateSetInfo_setExerciseId(setId, exerciseId);
			Database.updateSetInfo_setNumber(setId, setNumber);
			Database.updateSetInfo_setReps(setId, setReps);
			Database.updateSetInfo_setWeight(setId, setWeight);
			Database.updateSetInfo_setNotes(setId, setNotes);
			Database.updateSetInfo_setRestTime(setId, setRestTime);*/
			//Database.updateSetInfo_setSubSets(setId, setSubSets); //Not applicable to this set type
		}
	}
} // Item
