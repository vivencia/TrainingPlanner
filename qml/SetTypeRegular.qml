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
	required property int exerciseIdx
	required property int setNumber
	required property string setType

	signal requestTimerDialogSignal(Item requester, var args)

	ColumnLayout {
		id: setLayout
		Layout.fillWidth: true

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + (setNumber + 1).toString() + "  -  " + mainwindow.setTypesModel[setType].text
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
				onClicked: itemManager.removeSetObject(setNumber, exerciseIdx);
			}
		}

		SetInputField {
			id: txtRestTime
			type: SetInputField.Type.TimeType
			availableWidth: setItem.width
			windowTitle: lblSetNumber.text
			visible: setNumber > 0

			onValueChanged: (str) => {
				tDayModel.setSetRestTime(setNumber, str, exerciseIdx);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setRestTime(setNumber, exerciseIdx);
			onEnterOrReturnKeyPressed: txtNSubSets.forceActiveFocus();
		}

		SetInputField {
			id: txtNReps
			type: SetInputField.Type.RepType
			availableWidth: setItem.width

			onValueChanged: (str) => {
				tDayModel.setSetReps(setNumber, str, exerciseIdx);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setReps(setNumber, exerciseIdx);
			onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
		}

		SetInputField {
			id: txtNWeight
			type: SetInputField.Type.WeightType
			availableWidth: setItem.width

			onValueChanged: (str) => {
				tDayModel.setSetWeight(setNumber, str, exerciseIdx);
				text = str;
			}

			Component.onCompleted: text = tDayModel.setWeight(setNumber, exerciseIdx);
		}

		Label {
			text: qsTr("Notes:")
			Layout.bottomMargin: 10
			font.bold: true

			RoundButton {
				id: btnShowHideNotes
				anchors.left: parent.right
				anchors.verticalCenter: parent.verticalCenter
				anchors.rightMargin: 20
				width: 25
				height: 25

				Image {
					id: img
					source: "qrc:/images/"+darkIconFolder+"down.png"
					width: 20
					height: 20
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
				}

				onClicked: {
					txtSetNotes.visible = !txtSetNotes.visible;
					img.source = txtSetNotes.visible ? "qrc:/images/"+darkIconFolder+"up.png" : "qrc:/images/"+darkIconFolder+"down.png"
				}
			}
		}
		TextField {
			id: txtSetNotes
			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 5
			Layout.bottomMargin: 10
			visible: false

			onTextEdited: tDayModel.setSetNotes(text, exerciseIdx);
			Component.onCompleted: text = tDayModel.setNotes(setNumber, exerciseIdx);
		}
	} // setLayout

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args);
	}
} // FocusScope
