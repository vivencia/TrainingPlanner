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
	signal addSubSet(int id)
	signal delSubSet(int id)

	Layout.fillWidth: true

	implicitHeight: 40
	implicitWidth: mainRow.width

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
			text: nReps.toString()
			type: SetInputField.Type.RepType
			nSetNbr: setNbr
			showLabel: false
			availableWidth: mainRow.width/3
			focus: true // makes FocusScope choose this Item to give focus to
			Layout.alignment: Qt.AlignCenter
			Layout.row: 1
			Layout.column: 0

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

		Label {
			text: rowIdx === 0 ? qsTr("Weight:") : ""
			Layout.alignment: Qt.AlignCenter
			Layout.row: 0
			Layout.column: 1
		}
		SetInputField {
			id: txtNWeight
			text: nWeight.toString()
			type: SetInputField.Type.WeightType
			nSetNbr: setNbr
			showLabel: false
			availableWidth: mainRow.width/3
			Layout.alignment: Qt.AlignCenter
			Layout.row: 1
			Layout.column: 1

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

			ToolButton {
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

				onClicked: addSubSet(rowIdx+1);
			} //bntInsertAnotherRow

			ToolButton {
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
