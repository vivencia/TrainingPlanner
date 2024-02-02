import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "jsfunctions.js" as JSF

Item {
	id: setItem
	property int setId: -1
	property int exerciseIdx
	property int tDayId
	property int setType: 4 //Constant
	property int setNumber
	property string setReps
	property string setWeight
	property int setSubSets: 0
	property string setRestTime: "00:00"
	property string setNotes: " "
	property var nextObject: null

	property string exerciseName2
	property bool bUpdateLists
	property var subSetList: []

	signal setRemoved(int nset)
	signal setChanged(int nset, string reps, string weight, int subsets, string resttime, string setnotes)
	signal secondExerciseNameChanged(string new_exercisename)

	property string strReps1
	property string strReps2
	property string strWeight1
	property string strWeight2

	implicitHeight: setLayout.implicitHeight
	width: parent.width
	Layout.fillWidth: true
	Layout.leftMargin: 5

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString() + qsTr("  -  Giant set")
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
		}

		TextField {
			id: txtExerciseName2
			text: exerciseName2
			font.bold: true
			font.pixelSize: AppSettings.fontSizeText
			readOnly: true
			wrapMode: Text.WordWrap
			width: setItem.width - 75
			height: 60
			Layout.minimumWidth: width
			Layout.maximumWidth: width
			Layout.minimumHeight: height
			Layout.maximumHeight: height
			Layout.leftMargin: 5
			Layout.rightMargin: 5
			Layout.topMargin: 0
			visible: setNumber === 0
			z: 1

			background: Rectangle {
				color: txtExerciseName2.readOnly ? "transparent" : "white"
				border.color: txtExerciseName2.readOnly ? "transparent" : "black"
				radius: 5
			}

			Keys.onReturnPressed: { //Alphanumeric keyboard
				btnEditExercise2.clicked();
				txtRestTime.forceActiveFocus();
			}

			onReadOnlyChanged: {
				if (!readOnly) {
					const idx = exerciseName2.indexOf(':'); //Remove the '2: ' from the name
					exerciseName2 = exerciseName2.substring(idx + 1, exerciseName2.length).trim();
				}
				else
					ensureVisible(0);
			}

			onActiveFocusChanged: {
				if (activeFocus)
					closeSimpleExerciseList();
			}

			onEditingFinished: {
				exerciseName2 = "2: " + text;
				secondExerciseNameChanged(exerciseName2);
			}

			RoundButton {
				id: btnRemoveExercise2
				anchors.left: txtExerciseName2.right
				anchors.verticalCenter: txtExerciseName2.verticalCenter
				height: 25
				width: 25

				z: 2
				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.fill: parent
				}

				onClicked: {
					removeSecondExercise();
				}
			} //btnRemoveExercise2

			RoundButton {
				id: btnEditExercise2
				anchors.left: btnRemoveExercise2.right
				anchors.verticalCenter: txtExerciseName2.verticalCenter
				height: 25
				width: 25
				z: 2

				Image {
					source: "qrc:/images/"+darkIconFolder+"edit.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}

				onClicked: {
					if (txtExerciseName2.readOnly) {
						txtExerciseName2.readOnly = false;
						requestSimpleExerciseList(setItem);
					}
					else {
						txtExerciseName2.readOnly = true;
						closeSimpleExerciseList();
					}
				}
			} //btnEditExercise2
		} //txtExerciseName2

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
				txtNReps1.forceActiveFocus();
			}
		}

		GridLayout {
			Layout.fillWidth: true
			Layout.topMargin: 10
			Layout.bottomMargin: 10
			Layout.leftMargin: setItem.width/5
			rows: 5
			columns: 2
			columnSpacing: 15
			rowSpacing: 5

			Label {
				id: lblExercise1
				text: qsTr("Exercise 1")
				width: setItem.width/2
				font.bold: true
				Layout.row: 0
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
			}

			Label {
				id: lblExercise2
				text: qsTr("Exercise 2")
				width: setItem.width/2
				font.bold: true
				Layout.row: 0
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
			}

			Label {
				text: qsTr("Reps:")
				Layout.row: 1
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNReps1
				text: strReps1
				type: SetInputField.Type.RepType
				nSetNbr: setNumber
				availableWidth: setItem.width/3
				alternativeLabels: ["","","",""]
				Layout.row: 2
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter

				onEnterOrReturnKeyPressed: {
					txtNWeight1.forceActiveFocus();
				}

				onValueChanged: (str, val) => {
					onTextEdited: changeRep(0, str);
				}
			}

			Label {
				text: qsTr("Reps:")
				Layout.row: 1
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNReps2
				text: strReps2
				type: SetInputField.Type.RepType
				nSetNbr: setNumber
				availableWidth: setItem.width/3
				alternativeLabels: ["","","",""]
				Layout.row: 2
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter

				onEnterOrReturnKeyPressed: {
					txtNWeight2.forceActiveFocus();
				}

				onValueChanged: (str, val) => {
					onTextEdited: changeRep(1, str);
				}
			}

			Label {
				text: qsTr("Weight") + AppSettings.weightUnit + ":"
				Layout.row: 3
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNWeight1
				text: strWeight1
				type: SetInputField.Type.WeightType
				nSetNbr: setNumber
				availableWidth: setItem.width/3
				alternativeLabels: ["","","",""]
				Layout.row: 4
				Layout.column: 0
				Layout.alignment: Qt.AlignCenter

				onEnterOrReturnKeyPressed: {
					txtNReps2.forceActiveFocus();
				}

				onValueChanged: (str, val) => {
					changeWeight(0, str);
				}
			}

			Label {
				text: qsTr("Weight") + AppSettings.weightUnit + ":"
				Layout.row: 3
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter
			}
			SetInputField {
				id: txtNWeight2
				text: strWeight2
				type: SetInputField.Type.WeightType
				nSetNbr: setNumber
				availableWidth: setItem.width/3
				alternativeLabels: ["","","",""]
				Layout.row: 4
				Layout.column: 1
				Layout.alignment: Qt.AlignCenter

				onEnterOrReturnKeyPressed: {
					if (nextObject !== null)
						nextObject.forceActiveFocus()
					else
						txtSetNotes.forceActiveFocus();
				}

				onValueChanged: (str, val) => {
					changeWeight(1, str);
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
			getVariables();

	} //ColumnLayout setLayout

	function getVariables() {
		var idx = setReps.indexOf('#')
		if (idx === -1) {
			setReps = "10#10"; //Random default values
			idx = 2;
		}
		strReps1 = setReps.substring(0, idx);
		strReps2 = setReps.substring(idx+1, setReps.length);

		idx = setWeight.indexOf('#')
		if (idx === -1) {
			setWeight = "50#50"; //Random default values
			idx = 2;
		}
		strWeight1 = setWeight.substring(0, idx);
		strWeight2 = setWeight.substring(idx+1, setReps.length);
	}

	function changeRep(idx, newRep) {
		const reps = setReps.split('#');
		if (reps[idx] !== newRep) {
			if (idx === 0)
				strReps1 = newRep
			else
				strReps2 = newRep
			setReps = strReps1 + '#' + strReps2;
			setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
		}
	}

	function changeWeight(idx, newWeight) {
		const weights = setWeight.split('#');
		if (weights[idx] !== newWeight) {
			if (idx === 0)
				strWeight1 = newWeight;
			else
				strWeight2 = newWeight;
			setWeight = strWeight1 + '#' + strWeight2;
			setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
		}
	}

	function changeExercise(name1, name2) {
		exerciseName2 = "2: " + name1 + ' - ' + name2;
		secondExerciseNameChanged(exerciseName2);
	}

	function removeSecondExercise() {
		exerciseName2 = qsTr("2: Add exercise");
		secondExerciseNameChanged(exerciseName2);
	}

	function updateTrainingDayId(newTDayId) {
		tDayId = newTDayId;
		setId = -1; //Force a new DB entry to be created when set is logged
	}

	function logSet() {
		if (setNotes === "")
			setNotes = " ";

		if (setId < 0) {
			let result = Database.newSetInfo(tDayId, exerciseIdx, setType, setNumber, setReps,
								setWeight, AppSettings.weightUnit, setSubSets, setRestTime, setNotes);
			setId = result.insertId;
		}
		else {
			Database.updateSetInfo(setId, exerciseIdx, setNumber, setReps, setWeight, setSubSets.toString(), setRestTime, setNotes);
		}
	}
} // Item
