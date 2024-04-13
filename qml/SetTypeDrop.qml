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
	required property int setType

	property var subSetList: []

	signal requestTimerDialogSignal(Item requester, var args)

	onFocusChanged: {
		if (focus) {
			if (subSetList.length > 0)
				subSetList[0].Object.forceActiveFocus();
		}
	}

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true
		spacing: 5

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString()
			font.bold: true
			Layout.bottomMargin: 10

			TPComboBox {
				id: cboSetType
				currentIndex: setType
				anchors.left: parent.right
				anchors.leftMargin: 10
				anchors.verticalCenter: parent.verticalCenter

				onActivated: (index)=> {
					if (index !== setType)
						itemManager.changeSetType(setNumber, exerciseIdx, index);
				}
			}

			RoundButton {
				id: btnRemoveSet
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: cboSetType.right
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
		}

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			text: tDayModel.setRestTime(setNumber, exerciseIdx);
			availableWidth: setItem.width
			windowTitle: lblSetNumber.text
			visible: setNumber > 0

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, exerciseIdx, str);
				text = str;
			}

			onEnterOrReturnKeyPressed: {
				if (subSetList.length > 0)
					subSetList[0].Object.forceActiveFocus();
			}
		}

		ColumnLayout {
			id: subSetsLayout
			Layout.fillWidth: true
			Layout.topMargin: 10
			Layout.bottomMargin: 20

			RowLayout {
				Layout.fillWidth: true

				Label {
					text: qsTr("Reps:")
					width: setItem.width/2
					font.bold: true
					Layout.alignment: Qt.AlignCenter
					Layout.maximumWidth: width
					Layout.minimumWidth: width
				}

				Label {
					text: qsTr("Weight:")
					width: setItem.width/2
					font.bold: true
					Layout.alignment: Qt.AlignCenter
					Layout.maximumWidth: width
					Layout.minimumWidth: width
				}
			}
		} //subSetsLayout

		SetNotesField {
			id: btnShowHideNotes
		}
	} // setLayout

	Component.onCompleted: {
		const nsubsets = tDayModel.setSubSets_int(setNumber, exerciseIdx);
		for (var i = 0; i < nsubsets; ++i)
			addSubSet(i, false);
	}

	function addSubSet(idx, bNew) {
		if (bNew)
			tDayModel.newSetSubSet(setNumber, exerciseIdx);

		var component = Qt.createComponent("RepsAndWeightRow.qml", Qt.Asynchronous);

		function finishCreation() {
			var rowSprite = component.createObject(subSetsLayout, { width:windowWidth, tDayModel:tDayModel, rowIdx:idx });
			subSetList.push({"Object" : rowSprite});
			rowSprite.delSubSet.connect(removeSubSet);
			rowSprite.addSubSet.connect(addSubSet);

			if (idx >= 1) {
				subSetList[idx-1].Object.bBtnAddEnabled = false;
				subSetList[idx-1].Object.nextRowObj = rowSprite;
			}
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
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

	function changeReps(new_value: string, idx: int) {
		if (idx < subSetList.length)
			subSetList[idx].Object.changeReps(new_value);
	}

	function changeWeight(new_value: string, idx: int) {
		if (idx < subSetList.length)
			subSetList[idx].Object.changeWeight(new_value);
	}
} // Item
