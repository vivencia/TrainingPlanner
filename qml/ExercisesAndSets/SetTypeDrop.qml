import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../"
import "../TPWidgets"

import com.vivenciasoftware.qmlcomponents

Item {
	id: setItem
	height: setLayout.height + 15
	implicitHeight: setLayout.implicitHeight + 15
	enabled: tDayModel.dayIsEditable
	Layout.fillWidth: true
	Layout.leftMargin: 5
	Layout.rightMargin: 5

	required property DBTrainingDayModel tDayModel
	required property int exerciseIdx
	required property int setNumber
	required property int setType

	property bool finishButtonVisible: false
	property bool finishButtonEnabled: false
	property string copyTypeButtonValue: ""
	property string copyTimeButtonValue: ""
	property string copyRepsButtonValue: ""
	property string copyWeightButtonValue: ""
	property bool setCompleted: tDayModel.setCompleted(setNumber, exerciseIdx)
	property var subSetList: []
	property var subSetComponent: null
	readonly property int controlWidth: setItem.width - 20

	signal requestTimerDialogSignal(Item requester, var args)
	signal exerciseCompleted(int exercise_idx)
	signal showRemoveSetMessage(int set_number, int exercise_idx)

	onFocusChanged: {
		if (focus) {
			if (subSetList.length > 0)
				subSetList[0].Object.forceActiveFocus();
		}
	}

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true
		Layout.bottomMargin: 5

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString()
			font.bold: true
			Layout.bottomMargin: 10

			TPComboBox {
				id: cboSetType
				currentIndex: setType
				enabled: !setCompleted
				model: AppSettings.setTypesModel

				anchors {
					left: parent.right
					leftMargin: 10
					verticalCenter: parent.verticalCenter
				}

				onActivated: (index)=> {
					if (index !== setType) {
						if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
							if (copyTypeButtonValue === cboSetType.textAt(index))
								copyTypeButtonValue = "";
							else if (copyTypeButtonValue === "")
								copyTypeButtonValue = tDayModel.setType(setNumber, exerciseIdx).toString();
						}
						itemManager.changeSetType(setNumber, exerciseIdx, index);
					}
				}
			}

			TPRoundButton {
				id: btnCopyValue
				visible: copyTypeButtonValue !== ""
				imageName: "copy-setvalue.png"
				width: 25
				height: 25

				anchors {
					verticalCenter: parent.verticalCenter
					left: cboSetType.right
				}

				onClicked: {
					itemManager.copyTypeValueIntoOtherSets(exerciseIdx, setNumber);
					copyTypeButtonValue = "";
				}
			}

			TPRoundButton {
				id: btnRemoveSet
				imageName: "remove.png"
				height: 30
				width: 30

				anchors {
					verticalCenter: parent.verticalCenter
					left: copyTypeButtonValue ? btnCopyValue.right : cboSetType.right
				}

				onClicked: showRemoveSetMessage(setNumber, exerciseIdx);
			}

			TPCheckBox {
				id: chkSetCompleted
				text: qsTr("Completed")
				textColor: "black"
				checked: setCompleted
				visible: !setCompleted
				height: 25
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: btnRemoveSet.right

				onCheckedChanged: {
					setCompleted = checked;
					tDayModel.setSetCompleted(setNumber, exerciseIdx, setCompleted);
				}
			}

			Image {
				id: imgCompleted
				source: "qrc:/images/set-completed.png"
				asynchronous: true
				fillMode: Image.PreserveAspectFit
				visible: setCompleted
				width: 30
				height: 30
				anchors {
					verticalCenter: parent.verticalCenter
					left: btnRemoveSet.right
					leftMargin: 40
				}

				MouseArea {
					anchors.fill: parent
					onClicked: setCompleted = false;
				}
			}
		}

		RowLayout {
			visible: setNumber > 0
			enabled: !setCompleted
			Layout.leftMargin: 5

			SetInputField {
				id: txtRestTime
				type: SetInputField.Type.TimeType
				text: tDayModel.setRestTime(setNumber, exerciseIdx);
				availableWidth: copyTimeButtonValue === "" ? controlWidth : controlWidth - 40
				windowTitle: lblSetNumber.text

				onValueChanged: (str) => {
					if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
						if (copyTimeButtonValue === str)
							copyTimeButtonValue = "";
						else if (copyTimeButtonValue === "")
							copyTimeButtonValue = tDayModel.setRestTime(setNumber, exerciseIdx);
					}
					tDayModel.setSetRestTime(setNumber, exerciseIdx, str);
				}

				onEnterOrReturnKeyPressed: {
					if (subSetList.length > 0)
						subSetList[0].Object.forceActiveFocus();
				}
			}

			TPRoundButton {
				id: btnCopyTimeValue
				visible: copyTimeButtonValue !== ""
				imageName: "copy-setvalue.png"
				Layout.minimumHeight: 30
				Layout.maximumHeight: 30
				Layout.minimumWidth: 30
				Layout.maximumWidth: 30
				Layout.alignment: Qt.AlignRight

				onClicked: {
					itemManager.copyTimeValueIntoOtherSets(exerciseIdx, setNumber);
					copyTimeButtonValue = "";
				}
			}
		}

		Pane {
			implicitWidth: controlWidth
			implicitHeight: (tDayModel.setSubSets_int(setNumber, exerciseIdx) + 1) * 35
			padding: 0
			clip: true
			Layout.leftMargin: 5
			Layout.topMargin: 5

			background: Rectangle {
				color: "transparent"
			}

			Label {
				id: lblReps
				text: qsTr("Reps:")
				width: controlWidth/2
				font.bold: true

				anchors {
					left: parent.left
					top: parent.top
				}
			}

			Label {
				text: qsTr("Weight:")
				width: controlWidth/2
				font.bold: true

				anchors {
					left: lblReps.right
					right: parent.right
					top: parent.top
				}
			}

			ColumnLayout {
				id: subSetsLayout
				enabled: !setCompleted
				spacing: 5
				width: controlWidth

				anchors {
					top: lblReps.bottom
					topMargin: 5
					left: parent.left
					right: parent.right
					bottom: parent.bottom
					bottomMargin: 10
				}
			}
		}

		SetNotesField {
			id: btnShowHideNotes
			text: tDayModel.setNotes(setNumber, exerciseIdx)
			enabled: !setCompleted
			width: controlWidth
			Layout.leftMargin: 5
			Layout.bottomMargin: 5
			onEditFinished: (new_text) => tDayModel.setSetNotes(setNumber, exerciseIdx, new_text);
		}

		TPButton {
			id: btnCompleteExercise
			text: qsTr("Exercise completed")
			visible: finishButtonVisible
			enabled: finishButtonEnabled
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				setLayout.enabled = false;
				exerciseCompleted(exerciseIdx);
			}
		}
	} // setLayout

	Component.onCompleted: tDayModel.saveWorkout.connect(function() { copyTypeButtonValue = false; });

	function init() {
		const nsubsets = tDayModel.setSubSets_int(setNumber, exerciseIdx);
		for (var i = 0; i < nsubsets; ++i)
			addSubSet(i, false);
	}

	function addSubSet(idx, bNew) {
		if (bNew)
			tDayModel.newSetSubSet(setNumber, exerciseIdx);

		if (!subSetComponent)
			subSetComponent = Qt.createComponent("qrc:/qml/ExercisesAndSets/RepsAndWeightRow.qml", Qt.Asynchronous);

		function finishCreation() {
			var rowSprite = subSetComponent.createObject(subSetsLayout, { tDayModel:tDayModel, rowIdx:idx });
			subSetList.push({"Object" : rowSprite});
			rowSprite.delSubSet.connect(removeSubSet);
			rowSprite.addSubSet.connect(addSubSet);

			if (idx >= 1) {
				subSetList[idx-1].Object.bBtnAddEnabled = false;
				subSetList[idx-1].Object.nextRowObj = rowSprite;
			}
		}

		if (subSetComponent.status === Component.Ready)
			finishCreation();
		else
			subSetComponent.statusChanged.connect(finishCreation);
	}

	function removeSubSet(idx) {
		let newSubSetList = new Array;
		for( var i = 0, x = 0; i < subSetList.length; ++i ) {
			if (i > idx) {
				subSetList[i].Object.rowIdx--;
				if (i > 0)
					subSetList[i-1].Object.nextRowObj = subSetList[i].Object;
			}
			if (i !== idx) {
				subSetList[i].Object.bBtnAddEnabled = false;
				newSubSetList[x] = subSetList[i];
				x++;
			}
		}
		subSetList[idx].Object.destroy();
		delete subSetList;
		subSetList = newSubSetList;
		const nsubsets = subSetList.length-1;
		subSetList[nsubsets].Object.bBtnAddEnabled = true;
		tDayModel.setSetSubSets(setNumber, exerciseIdx, nsubsets.toString());
	}

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}

	function changeSetType(new_type: int) {
		cboSetType.currentIndex = new_type;
	}

	function changeTime(new_time: string) {
		txtRestTime.text = new_time;
	}

	function changeReps(new_value: string, idx: int) {
		if (idx < subSetList.length)
			subSetList[idx].Object.changeReps(new_value);
	}

	function changeWeight(new_value: string, idx: int) {
		if (idx < subSetList.length)
			subSetList[idx].Object.changeWeight(new_value);
	}
} // Item
