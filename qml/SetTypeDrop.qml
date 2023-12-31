import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "jsfunctions.js" as JSF

Item {
	id: setItem
	property int setId
	property int exerciseId
	property int tDayId
	property int setType: 2 //Constant
	property int setNumber
	property string setReps
	property string setWeight
	property string setWeightUnit
	property int setSubSets
	property string setRestTime: "00:00"
	property string setNotes: " "

	property bool bIsRemoving: false
	property bool bUpdateLists
	property var subSetList: []

	signal setRemoved(int nset)
	signal setChanged(int nset, string reps, string weight, int subsets, string resttime, string setnotes)

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
		spacing: 0

		Label {
			id: lblSetNumber
			text: qsTr("Set #: ") + (setNumber + 1).toString() + qsTr("  -  Drop set")
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

		RowLayout {
			Layout.fillWidth: true
			Layout.topMargin: 20
			Layout.bottomMargin: 10

			Label {
				text: qsTr("Reps:")
				Layout.alignment: Qt.AlignCenter
			}
			Rectangle {
				Layout.minimumWidth: 100
			}
			Label {
				text: qsTr("Weight:")
				Layout.alignment: Qt.AlignCenter
			}
		}

		ColumnLayout {
			id: subSetsLayout
			Layout.fillWidth: true
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
	} // setLayout

	Component.onCompleted: {
		for (var i = 1; i <= setSubSets; ++i) {
			addSubSet(i-1, false);
		}
	}

	function getReps(idx) {
		if (setReps.length > 0) {
			const reps = setReps.split(',');
			return parseInt(reps[idx]);
		}
		return "10"; //Random default value
	}

	function getWeight(idx) {
		if (setWeight.length > 0) {
			const weights = setWeight.split(',');
			return parseInt(weights[idx]);
		}
		return "100"; //Random default value
	}

	function subSetAdded(newReps, newWeight) {
		setSubSets++;
		setReps += ',' + newReps.toString();
		setWeight += ',' + newWeight.toString();
		setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
	}

	function removeSet(idx) {
		const reps = setReps.split(',');
		const weights = setWeight.split(',');
		setReps = "";
		setWeight = "";
		for(var i = 0; i < reps.length; ++i) { //Both arrays are always the same length
			if (i !== idx) {
				setReps += ',' + reps[i];
				setWeight += ',' + weights[i];
			}
		}
		setRemoved(idx);
		setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
	}

	function addSubSet(idx, bCreateNew) {
		var component;
		var rowSprite;
		component = Qt.createComponent("RepsAndWeightRow.qml");
		if (component.status === Component.Ready) {
			if (idx >= 1) {
				if (bCreateNew)
					subSetAdded(parseInt(getReps(idx-1)) * 0.8, parseInt(getWeight(idx-1)) * 0.6);
			}
			rowSprite = component.createObject(subSetsLayout, {
						rowIdx:idx, nReps:getReps(idx), nWeight:getWeight(idx), setNbr:setNumber
			});
			subSetList.push({"Object" : rowSprite});
			rowSprite.delSubSet.connect(removeSubSet);
			rowSprite.addSubSet.connect(addSubSet);
			rowSprite.changeSubSet.connect(subSetChanged);

			if (idx >= 1) {
				subSetList[idx-1].Object.bBtnAddEnabled = false;
				subSetList[idx-1].Object.nextRowObj = rowSprite;
			}
		}
	}

	function removeSubSet(idx) {
		let newSubSetList = new Array;
		subSetList[idx].Object.destroy();
		setSubSets--;
		for( var i = 0, x = 0; i < subSetList.length; ++i ) {
			if (i >= idx)
				subSetList[i].Object.rowIdx--;
			if (i !== idx) {
				subSetList[i].Object.bBtnAddEnabled = false;
				newSubSetList[x] = subSetList[i];
				x++;
			}
		}
		delete subSetList;
		subSetList = newSubSetList;
		subSetList[subSetList.length-1].Object.bBtnAddEnabled = true;
	}

	function subSetChanged(idx, newreps, newweight) {
		const reps = setReps.split(',');
		const weights = setWeight.split(',');
		setReps = "";
		setWeight = "";
		for(var i = 0; i < reps.length; ++i) {
			if (i !== idx) {
				setReps += ',' + reps[i];
				setWeight += ',' + weights[i];
			}
			else {
				setReps += ',' + newreps.toString();
				setWeight += ',' + newweight.toString();
			}
		}
		setChanged(setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes);
	}

	function updateTrainingDayId(newTDayId) {
		tDayId = newTDayId;
		setId = -1; //Force a new DB entry to be created when set is logged
	}

	function logSet() {
		if (setNotes === "")
			setNotes = " ";

		if (setId < 0) {
			console.log("SetTypeDropSet: #" + setNumber.toString() + "  " + exerciseId.toString() + "   " + tDayId.toString());
			let result = Database.newSetInfo(tDayId, exerciseId, setType, setNumber, setReps,
								setWeight, AppSettings.weightUnit, setSubSets, setRestTime, setNotes);
			setId = result.insertId;
		}
		else {
			Database.updateSetInfo(setId, exerciseId, setNumber, setReps, setWeight, setSubSets.toString(), setRestTime, setNotes);
			/*Database.updateSetInfo_setNumber(setId, setNumber);
			Database.updateSetInfo_setReps(setId, setReps);
			Database.updateSetInfo_setWeight(setId, setWeight);
			Database.updateSetInfo_setNotes(setId, setNotes);
			Database.updateSetInfo_setRestTime(setId, setRestTime);
			Database.updateSetInfo_setSubSets(setId, setSubSets);*/
		}
	}
} // Item
