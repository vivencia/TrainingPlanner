import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

FocusScope {
	required property DBTrainingDayModel tDayModel
	required property int rowIdx

	property var nextRowObj: null
	property bool bBtnAddEnabled: true

	signal addSubSet(int id, bool bnew)
	signal delSubSet(int id)

	Layout.fillWidth: true
	height: 40

	GridLayout {
		id: mainRow
		anchors.fill: parent
		rows: 2
		columns: 3
		rowSpacing: 0

		Label {
			text: rowIdx === 0 ? qsTr("Reps:") : ""
			Layout.alignment: Qt.AlignCenter
			Layout.row: 0
			Layout.column: 0
		}
		SetInputField {
			id: txtNReps
			type: SetInputField.Type.RepType
			showLabel: false
			availableWidth: mainRow.width/3
			focus: true // makes FocusScope choose this Item to give focus to
			Layout.alignment: Qt.AlignCenter
			Layout.row: 1
			Layout.column: 0

			onValueChanged: (str) => {
				tDayModel.setSetReps(setNumber, exerciseIdx, rowIdx, str);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setReps(setNumber, rowIdx, exerciseIdx);
			onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
		}

		Label {
			text: rowIdx === 0 ? qsTr("Weight:") : ""
			Layout.alignment: Qt.AlignCenter
			Layout.row: 0
			Layout.column: 1
		}
		SetInputField {
			id: txtNWeight
			type: SetInputField.Type.WeightType
			showLabel: false
			availableWidth: mainRow.width/3
			Layout.alignment: Qt.AlignCenter
			Layout.row: 1
			Layout.column: 1

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
			}

			Component.onCompleted: text = tDayModel.setWeight(setNumber, rowIdx, exerciseIdx);

			RoundButton {
				id: btnInsertAnotherRow
				width: 25
				height: 25
				visible: bBtnAddEnabled
				anchors {
					left: txtNWeight.right
					verticalCenter: txtNWeight.verticalCenter
					leftMargin: 3
				}

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
				anchors {
					left: btnInsertAnotherRow.right
					verticalCenter: txtNWeight.verticalCenter
					leftMargin: 3
				}

				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.fill: parent
					height: 20
					width: 20
				}

				onClicked: delSubSet(rowIdx);
			} //btnRemoveRow
		} //txtNWeight

		Rectangle {
			width: 60
			Layout.row: 1
			Layout.column: 2
		}
	} //GridLayout
} //FocusScope
