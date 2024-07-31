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
			tDayModel.setSetReps(setNumber, exerciseIdx, rowIdx, str);
			if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
				stack1.currentIndex = 1;
		}

		onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
	}

	StackLayout {
		id: stack1
		currentIndex: 0
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

		TPRoundButton {
			id: btnCopyValue
			width: parent.width
			height: parent.height
			imageName: "copy-setvalue.png"

			onClicked: {
				itemManager.copyRepsValueIntoOtherSets(exerciseIdx, setNumber, rowIdx);
				stack1.currentIndex = 0;
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
			tDayModel.setSetWeight(setNumber, exerciseIdx, rowIdx, str);
			if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
					stack2.currentIndex = 1;
		}
	} //txtNWeight

	StackLayout {
		id: stack2
		currentIndex: 0
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

			TPRoundButton {
				id: btnRemoveRow
				width: 25
				height: 25
				visible: rowIdx > 0
				imageName: "remove.png"

				anchors {
					left: parent.left
					leftMargin: -5
					verticalCenter: parent.verticalCenter
				}

				onClicked: delSubSet(rowIdx);
			} //btnRemoveRow

			TPRoundButton {
				id: btnInsertAnotherRow
				width: 25
				height: 25
				visible: bBtnAddEnabled
				imageName: "add-new.png"

				anchors {
					left: btnRemoveRow.right
					leftMargin: -10
					verticalCenter: parent.verticalCenter
				}

				onClicked: addSubSet(rowIdx+1, true);
			} //bntInsertAnotherRow
		}

		TPRoundButton {
			id: btnCopyValue2
			width: 30
			height: parent.height
			imageName: "copy-setvalue.png"

			onClicked: {
				itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, rowIdx);
				stack2.currentIndex = 0;
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
