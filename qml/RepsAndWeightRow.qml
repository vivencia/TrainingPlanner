import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

FocusScope {
	required property int nReps
	required property int nWeight
	required property int rowIdx
	property int setNbr
	property var nextRowObj: null

	property bool bBtnAddEnabled: true

	signal changeSubSet(int id, int reps, int weight)
	signal addSubSet(int id, bool bnew)
	signal delSubSet(int id)

	Layout.fillWidth: true

	implicitHeight: 40
	implicitWidth: mainRow.width

	Row {
		id: mainRow
		anchors.fill: parent

		SetInputField {
			id: txtNReps
			text: nReps.toString()
			type: SetInputField.Type.RepType
			nSetNbr: setNbr
			showLabel: false
			availableWidth: mainRow.width/2 - 30
			focus: true // makes FocusScope choose this Item to give focus to

			onEnterOrReturnKeyPressed: {
				txtNWeight.forceActiveFocus();
			}

			onValueChanged: (str, val) => {
				if (val !== nReps) {
					nReps = val;
					changeSubSet(rowIdx, nReps, nWeight);
				}
			}
		}

		SetInputField {
			id: txtNWeight
			text: nWeight.toString()
			type: SetInputField.Type.WeightType
			nSetNbr: setNbr
			showLabel: false
			availableWidth: mainRow.width/2 - 30

			onEnterOrReturnKeyPressed: {
				if (nextRowObj !== null)
					nextRowObj.forceActiveFocus();
			}

			onValueChanged: (str, val) => {
				if (val !== nWeight) {
					nWeight = val;
					changeSubSet(rowIdx, nReps, nWeight);
				}
			}
		}

		ToolButton {
			id: btnInsertAnotherRow
			width: 25
			height: 25
			visible: bBtnAddEnabled
			Layout.maximumHeight: 25
			Layout.maximumWidth: 25
			Layout.alignment: Qt.AlignCenter

			Image {
				source: "qrc:/images/"+darkIconFolder+"add-new.png"
				anchors.fill: parent
			}

			onClicked: addSubSet(rowIdx+1, true);
		}

		ToolButton {
			id: btnRemoveRow
			width: 25
			height: 25
			visible: rowIdx > 0
			Layout.maximumHeight: 25
			Layout.maximumWidth: 25
			Layout.alignment: Qt.AlignCenter

			Image {
				source: "qrc:/images/"+darkIconFolder+"remove.png"
				anchors.fill: parent
			}

			onClicked: {
				delSubSet(rowIdx);
			}
		}
	} //Row
} //FocusScope
