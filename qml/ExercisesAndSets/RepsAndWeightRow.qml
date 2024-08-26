import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../TPWidgets"
import com.vivenciasoftware.qmlcomponents

Item {
	id: mainRow
	implicitWidth: controlWidth
	implicitHeight: 30

	required property DBTrainingDayModel tDayModel
	required property int rowIdx
	readonly property int controlWidth: windowWidth
	property var nextRowObj: null
	property bool bBtnAddEnabled: true

	signal addSubSet(int row, bool bnew)
	signal delSubSet(int row)

	SetInputField {
		id: txtNReps
		type: SetInputField.Type.RepType
		text: tDayModel.setReps(setNumber, rowIdx, exerciseIdx);
		availableWidth: controlWidth/3
		showLabel: false

		anchors {
			left: parent.left
			top: parent.top
			bottom: parent.bottom
		}

		onValueChanged: (str) => {
			if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
				if (copyRepsButtonValue === str)
					copyRepsButtonValue = "";
				else if (copyRepsButtonValue === "")
					copyRepsButtonValue = tDayModel.setReps(setNumber, rowIdx, exerciseIdx);
			}
			tDayModel.setSetReps(setNumber, exerciseIdx, rowIdx, str);
		}

		onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
	}

	StackLayout {
		id: stack1
		currentIndex: copyRepsButtonValue !== "" ? 1 : 0
		width: 30
		height: parent.height

		anchors {
			left: txtNReps.right
			verticalCenter: parent.verticalCenter
		}

		Item {
			width: parent.width
			height: parent.height
		}

		TPButton {
			id: btnCopyValue
			imageSource: "copy-setvalue"
			height: 25
			width: 25

			onClicked: {
				itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, rowIdx);
				copyRepsButtonValue = "";
			}
		}
	}

	SetInputField {
		id: txtNWeight
		type: SetInputField.Type.WeightType
		text: tDayModel.setWeight(setNumber, rowIdx, exerciseIdx);
		availableWidth: controlWidth/3
		implicitHeight: 30
		showLabel: false

		anchors {
			left: stack1.right
			verticalCenter: parent.verticalCenter
		}

		onEnterOrReturnKeyPressed: {
			if (nextRowObj !== null)
				nextRowObj.forceActiveFocus();
			else {
				const nextSet = itemManager.nextSetObject(exerciseIdx, setNumber);
				if (nextSet)
					nextSet.forceActiveFocus();
			}
		}

		onValueChanged: (str) => {
			if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1) {
				if (copyWeightButtonValue === str)
					copyWeightButtonValue = "";
				else if (copyWeightButtonValue === "")
					copyWeightButtonValue = tDayModel.setWeight(setNumber, rowIdx, exerciseIdx);
			}
			tDayModel.setSetWeight(setNumber, exerciseIdx, rowIdx, str);
		}
	} //txtNWeight

	StackLayout {
		id: stack2
		currentIndex: copyWeightButtonValue !== "" ? 1 : 0
		width: 85
		implicitWidth: 85
		height: parent.height

		anchors {
			left: txtNWeight.right
			verticalCenter: parent.verticalCenter
		}

		Item {
			implicitWidth: 85
			implicitHeight: 30

			TPButton {
				id: btnRemoveRow
				imageSource: "remove"
				visible: rowIdx > 0
				width: 25
				height: 25

				anchors {
					left: parent.left
					leftMargin: -5
					verticalCenter: parent.verticalCenter
				}

				onClicked: delSubSet(rowIdx);
			} //btnRemoveRow

			TPButton {
				id: btnInsertAnotherRow
				imageSource: "add-new"
				visible: bBtnAddEnabled
				width: 25
				height: 25

				anchors {
					left: btnRemoveRow.right
					leftMargin: -5
					verticalCenter: parent.verticalCenter
				}

				onClicked: addSubSet(rowIdx+1, true);
			} //bntInsertAnotherRow
		}

		TPButton {
			id: btnCopyValue2
			imageSource: "copy-setvalue"
			height: 25
			width: 25

			onClicked: {
				itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, rowIdx);
				copyWeightButtonValue = "";
			}
		}
	}

	Component.onCompleted: tDayModel.saveWorkout.connect(hideCopyButtons);

	function hideCopyButtons() {
		stack1.currentIndex = 0;
		stack2.currentIndex = 0;
	}

	function changeReps(new_value: string) {
		txtNReps.text = new_value;
	}

	function changeWeight(new_value: string) {
		txtNWeight.text = new_value;
	}
} //RowLayout
