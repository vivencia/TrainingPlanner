import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

RowLayout {
	id: mainRow
	required property DBTrainingDayModel tDayModel
	required property int rowIdx

	property var nextRowObj: null
	property bool bBtnAddEnabled: true

	signal addSubSet(int id, bool bnew)
	signal delSubSet(int id)

	Layout.fillWidth: true
	spacing: 0
	height: 40

	SetInputField {
		id: txtNReps
		type: SetInputField.Type.RepType
		text: tDayModel.setReps(setNumber, rowIdx, exerciseIdx);
		availableWidth: windowWidth/4 + 10
		showLabel: false

		onValueChanged: (str) => {
			tDayModel.setSetReps(setNumber, exerciseIdx, rowIdx, str);
			text = str;
			if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
				stack1.currentIndex = 1;
		}

		onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
	}

	StackLayout {
		id: stack1
		currentIndex: 0

		Item {
			width: 30
			height: 40
		}

		RoundButton {
			id: btnCopyValue
			Layout.maximumWidth: 40
			Layout.minimumWidth: 40
			Layout.maximumHeight: 40
			Layout.minimumHeight: 40

			Image {
				source: "qrc:/images/"+darkIconFolder+"copy-setvalue.png"
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenter: parent.horizontalCenter
				height: 20
				width: 20
			}

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
		availableWidth: windowWidth/4 + 10
		showLabel: false

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
			text = str;
			if (setNumber < tDayModel.setsNumber(exerciseIdx) - 1)
					stack2.currentIndex = 1;
		}
	} //txtNWeight

	StackLayout {
		id: stack2
		currentIndex: 0

		Row {

			RoundButton {
				id: btnInsertAnotherRow
				width: 25
				height: 25
				visible: bBtnAddEnabled

				Image {
					source: "qrc:/images/"+darkIconFolder+"add-new.png"
					anchors.fill: parent
					width: 20
					height: 20
				}

				onClicked: addSubSet(rowIdx+1, true);
			} //bntInsertAnotherRow

			RoundButton {
				id: btnRemoveRow
				width: 25
				height: 25
				visible: rowIdx > 0

				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.fill: parent
					height: 20
					width: 20
				}

				onClicked: delSubSet(rowIdx);
			} //btnRemoveRow
		}

		RoundButton {
			id: btnCopyValue2
			Layout.maximumWidth: 40
			Layout.minimumWidth: 40
			Layout.maximumHeight: 40
			Layout.minimumHeight: 40

			Image {
				source: "qrc:/images/"+darkIconFolder+"copy-setvalue.png"
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenter: parent.horizontalCenter
				height: 20
				width: 20
			}

			onClicked: {
				itemManager.copyWeightValueIntoOtherSets(exerciseIdx, setNumber, rowIdx);
				stack2.currentIndex = 0;
			}
		}
	}

	Component.onCompleted: tDayModel.modifiedChanged.connect(hideCopyButtons);

	function hideCopyButtons() {
		if (!tDayModel.modified) {
			stack1.currentIndex = 0;
			stack2.currentIndex = 0;
		}
	}

	function changeReps(new_value: string) {
		txtNReps.text = new_value;
	}

	function changeWeight(new_value: string) {
		txtNWeight.text = new_value;
	}
} //RowLayout
