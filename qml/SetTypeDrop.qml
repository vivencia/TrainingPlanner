import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

Item {
	id: setItem
	implicitHeight: setLayout.implicitHeight + 10
	Layout.fillWidth: true
	Layout.leftMargin: 5

	required property DBTrainingDayModel tDayModel
	required property int exerciseIdx
	required property int setNumber
	required property int setType

	property bool finishButtonVisible: false
	property bool finishButtonEnabled: false
	property bool setCompleted: tDayModel.setCompleted(setNumber, exerciseIdx)
	property var subSetList: []
	property var subSetComponent: null

	signal requestTimerDialogSignal(Item requester, var args)
	signal exerciseCompleted(int exercise_idx)

	onFocusChanged: {
		if (focus) {
			if (subSetList.length > 0)
				subSetList[0].Object.forceActiveFocus();
		}
	}

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Set")
		message: lblSetNumber.text + qsTr("? This action cannot be undone.")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		onButton1Clicked: itemManager.removeSetObject(setNumber, exerciseIdx);
	} //TPBalloonTip

	ColumnLayout {
		id: setLayout
		spacing: 5
		Layout.fillWidth: true
		Layout.bottomMargin: 10

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString()
			font.bold: true
			Layout.bottomMargin: 10

			TPComboBox {
				id: cboSetType
				currentIndex: setType
				enabled: !setCompleted

				anchors {
					left: parent.right
					leftMargin: 10
					verticalCenter: parent.verticalCenter
				}

				onActivated: (index)=> {
					if (index !== setType)
						itemManager.changeSetType(setNumber, exerciseIdx, index);
				}
			}

			TPRoundButton {
				id: btnRemoveSet
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: cboSetType.right
				height: 25
				width: 25
				imageName: "remove.png"

				onClicked: {
					if (AppSettings.alwaysAskConfirmation)
						msgDlgRemove.show(-1);
					else
						itemManager.removeSetObject(setNumber, exerciseIdx);
				}
			}

			TPCheckBox {
				id: chkSetCompleted
				text: qsTr("Completed?")
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
				source: "qrc:/images/"+darkIconFolder+"set-completed.png"
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

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			text: tDayModel.setRestTime(setNumber, exerciseIdx);
			availableWidth: setItem.width
			windowTitle: lblSetNumber.text
			visible: setNumber > 0
			enabled: !setCompleted

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
			enabled: !setCompleted
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
			text: tDayModel.setNotes(setNumber, exerciseIdx)
			enabled: !setCompleted
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

	Component.onCompleted: {
		const nsubsets = tDayModel.setSubSets_int(setNumber, exerciseIdx);
		for (var i = 0; i < nsubsets; ++i)
			addSubSet(i, false);
	}

	function addSubSet(idx, bNew) {
		if (bNew)
			tDayModel.newSetSubSet(setNumber, exerciseIdx);

		if (!subSetComponent)
			subSetComponent = Qt.createComponent("RepsAndWeightRow.qml", Qt.Asynchronous);

		function finishCreation() {
			var rowSprite = subSetComponent.createObject(subSetsLayout, { width:windowWidth, tDayModel:tDayModel, rowIdx:idx });
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

	function changeReps(new_value: string, idx: int) {
		if (idx < subSetList.length)
			subSetList[idx].Object.changeReps(new_value);
	}

	function changeWeight(new_value: string, idx: int) {
		if (idx < subSetList.length)
			subSetList[idx].Object.changeWeight(new_value);
	}
} // Item
