import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "jsfunctions.js" as JSF

Item {
	id: setItem
	property int setId
	property int exerciseIdx
	property int tDayId
	property int setType: 4 //Constant
	property int setNumber
	property string setReps
	property string setWeight
	property int setSubSets: 0
	property string setRestTime: "00:00"
	property string setNotes: " "

	property string exerciseName2
	property bool bIsRemoving: false
	property bool bUpdateLists
	property var subSetList: []
	property var stackViewObj

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
			text: qsTr("Set #") + (setNumber + 1).toString() + qsTr("  -  Giant set")
			font.bold: true

			ToolButton {
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

			ToolButton {
				id: btnRemoveExercise2
				anchors.left: txtExerciseName2.right
				anchors.verticalCenter: txtExerciseName2.verticalCenter
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
				anchors.left: btnRemoveExercise2.right
				anchors.verticalCenter: txtExerciseName2.verticalCenter
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
				availableWidth: setItem.width/4 - 10
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
				availableWidth: setItem.width/4 - 10
				alternativeLabels: ["","","",""]

				onEnterOrReturnKeyPressed: {
					txtNWeight2.forceActiveFocus();
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
			getVariables();

	} //ColumnLayout setLayout

	function getVariables() {
		var idx = setReps.indexOf('|')
		if (idx === -1) {
			setReps = "10|10"; //Random default values
			idx = 2;
		}
		strReps1 = setReps.substring(0, idx);
		strReps2 = setReps.substring(idx+1, setReps.length);

		idx = setWeight.indexOf('|')
		if (idx === -1) {
			setWeight = "50|50"; //Random default values
			idx = 2;
		}
		strWeight1 = setWeight.substring(0, idx);
		strWeight2 = setWeight.substring(idx+1, setReps.length);
	}

	function changeRep(idx, newRep) {
		const reps = setReps.split('|');
		if (reps[idx] !== newRep) {
			if (idx === 0)
				strReps1 = newRep
			else
				strReps2 = newRep
			setReps = strReps1 + "|" + strReps2;
			setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
		}
	}

	function changeWeight(idx, newRep) {
		const weights = setWeight.split('|');
		if (weights[idx] !== newRep) {
			if (idx === 0)
				strWeight1 = newRep;
			else
				strWeight2 = newRep;
			setWeight = strWeight1 + '|' + strWeight2;
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
			console.log("SetGiantDropSet: #" + setNumber.toString() + "  " + exerciseIdx.toString() + "   " + tDayId.toString());
			let result = Database.newSetInfo(tDayId, exerciseIdx, setType, setNumber, setReps,
								setWeight, AppSettings.weightUnit, setSubSets, setRestTime, setNotes);
			setId = result.insertId;
		}
		else {
			Database.updateSetInfo(setId, exerciseIdx, setNumber, setReps, setWeight, setSubSets.toString(), setRestTime, setNotes);
		}
	}
} // Item
