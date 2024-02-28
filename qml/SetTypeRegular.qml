import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

FocusScope {
	id: setItem
	implicitHeight: setLayout.implicitHeight
	Layout.fillWidth: true
	Layout.leftMargin: 5
	Layout.rightMargin: 5

	required property DBTrainingDayModel tDayModel
	required property int setNumber
	readonly property int setType: 0 //Constant

	signal setRemoved(int set_number)
	property var nextObject: null

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString() + qsTr("  -  Regular set")
			font.bold: true

			RoundButton {
				id: btnRemoveSet
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.right
				height: 25
				width: 25

				Image {
					source: "qrc:/images/"+darkIconFolder+"remove.png"
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
					height: 20
					width: 20
				}
				onClicked: {
					if (tDayModel.removeSet(setNumber))
						setRemoved(setNumber);
				}
			}
		}

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			availableWidth: setItem.width
			nSetNbr: setNumber
			text: setNumber !== 0 ? tDayModel.setRestTime(setNumber) : "00:00"
			windowTitle: lblSetNumber.text
			focus: setNumber !== 0

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, str)
			}

			onEnterOrReturnKeyPressed: {
				txtNReps.forceActiveFocus();
			}
		}

		SetInputField {
			id: txtNReps
			text: tDayModel.setReps(setNumber)
			type: SetInputField.Type.RepType
			nSetNbr: setNumber
			availableWidth: setItem.width

			onEnterOrReturnKeyPressed: {
				txtNWeight.forceActiveFocus();
			}

			onValueChanged: (str) => {
				tDayModel.setSetReps(setNumber, str);
			}
		}

		SetInputField {
			id: txtNWeight
			text: tDayModel.setWeight(setNumber)
			type: SetInputField.Type.WeightType
			nSetNbr: setNumber
			availableWidth: setItem.width

			onEnterOrReturnKeyPressed: {
				if (nextObject !== null)
					nextObject.forceActiveFocus()
				else
					txtSetNotes.forceActiveFocus();
			}

			onValueChanged: (str) => {
				tDayModel.setSetWeight(setNumber, str);
			}
		}

		Label {
			text: qsTr("Notes:")
			Layout.topMargin: 10
			padding: 0
		}
		TextField {
			id: txtSetNotes
			text: tDayModel.setNotes(setNumber)
			font.bold: true
			Layout.fillWidth: true
			Layout.leftMargin: 10
			Layout.rightMargin: 10
			padding: 0

			onTextEdited: {
				tDayModel.setSetNotes(text);
			}
		}
	} // setLayout
} // FocusScope
