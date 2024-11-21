import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

FocusScope {
	id: setItem
	implicitHeight: setLayout.implicitHeight + 20
	enabled: setManager.isEditable
	Layout.fillWidth: true

	required property SetEntryManager setManager
	required property ExerciseEntryManager exerciseManager

	readonly property int controlWidth: setItem.width - 20
	readonly property var myoLabels: setManager.type === 5 ? [ qsTr("Weight:"), setManager.number === 0 ? qsTr("Reps to failure:") : qsTr("Reps to match:"),
						qsTr("Rest time:"), qsTr("Number of short rest pauses:") ] : []

	Connections {
		target: setManager
			function ontypeChanged() { btnCopySetType.visible = !btnCopySetType.visible; }
			function onRestTimeChanged() { btnCopyTimeValue.visible = !btnCopyTimeValue.visible; }
			function onReps1Changed() { btnCopySetReps.visible = !btnCopySetReps.visible; }
			function onWeight1Changed() { btnCopySetWeight.visible = !btnCopySetWeight.visible; }
	}

	Rectangle {
		id: indicatorRec
		visible: false
		color: appSettings.entrySelectedColor
		layer.enabled: true
		radius: 5
		border.color: "#707d8d"
		border.width: 1
		anchors.fill: parent
	}

	MultiEffect {
		id: currentSetEffect
		visible: setManager.current
		source: indicatorRec
		shadowEnabled: true
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1
		opacity: 0.5
		anchors.fill: parent
	}

	ColumnLayout {
		id: setLayout
		anchors.fill: parent

		Item {
			height: 30
			Layout.fillWidth: true

			TPButton {
				id: btnManageSet
				text: setManager.modeLabel
				flat: false
				visible: !setManager.completed
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked: exerciseManager.changeSetMode(setManager.number);
			}

			TPButton {
				id: imgCompleted
				imageSource: "set-completed"
				visible: setManager.completed
				height: 30
				width: 30
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked: exerciseManager.changeSetMode(setManager.number);
			}
		}

		Label {
			id: lblSetNumber
			text: qsTr("Set #") + setManager.strNumber
			font.bold: true

			TPComboBox {
				id: cboSetType
				currentIndex: setManager.type
				enabled: !setManager.completed
				model: AppGlobals.setTypesModel
				implicitWidth: 160

				anchors {
					left: parent.right
					leftMargin: 10
					verticalCenter: parent.verticalCenter
				}

				onActivated: (index)=> exerciseManager.changeSetType(setManager.number, index);
			}

			TPButton {
				id: btnCopySetType
				visible: false
				imageSource: "copy-setvalue"
				height: 25
				width: 25

				anchors {
					verticalCenter: parent.verticalCenter
					left: cboSetType.right
					leftMargin: 10
				}

				onClicked: exerciseManager.copyTypeValueIntoOtherSets(setManager.number);
			}

			TPButton {
				id: btnRemoveSet
				imageSource: "remove"
				height: 20
				width: 20

				anchors {
					verticalCenter: parent.verticalCenter
					left: btnCopySetType.visible ? btnCopySetType.right : cboSetType.right
					leftMargin: 10
				}

				onClicked: exerciseManager.removeSetObject(setManager.number);
			}
		}

		Row {
			visible: setManager.number > 0 && setManager.trackRestTime
			enabled: !setManager.completed && !setManager.autoRestTime

			SetInputField {
				id: txtRestTime
				type: SetInputField.Type.TimeType
				text: setManager.restTime
				availableWidth: btnCopyTimeValue.visible ? controlWidth - 40 : controlWidth
				showButtons: !setManager.autoRestTime

				onValueChanged: (str) => setManager.restTime = str;

				onEnterOrReturnKeyPressed: {
					if (txtNSubSets.visible)
						txtNSubSets.forceActiveFocus();
					else
						txtNReps.forceActiveFocus();
				}
			}

			TPButton {
				id: btnCopyTimeValue
				visible: false
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: exerciseManager.copyTimeValueIntoOtherSets(setManager.number);
			}
		}

		SetInputField {
			id: txtNSubSets
			type: SetInputField.SetType
			text: setManager.subSets
			availableWidth: controlWidth
			visible: setManager.hasSubSets
			alternativeLabels: myoLabels
			enabled: !setManager.completed

			onValueChanged: (str) => setManager.subSets = str;

			onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();

			TPLabel {
				id: lblTotalReps
				text: setManager.strTotalReps
				visible: setManager.hasSubSets

				anchors {
					top: parent.verticalCenter
					topMargin: -(height/2)
					left: parent.horizontalCenter
					leftMargin: 10
				}
			}
		}

		Row {
			enabled: !setManager.completed
			Layout.fillWidth: true

			SetInputField {
				id: txtNReps
				type: SetInputField.Type.RepType
				text: setManager.reps1
				availableWidth: btnCopySetReps.visible ? controlWidth - 40 : controlWidth
				alternativeLabels: myoLabels

				onValueChanged: (str) => setManager.reps1 = str;
				onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
			}

			TPButton {
				id: btnCopySetReps
				visible: false
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: exerciseManager.copyRepsValueIntoOtherSets(setManager.number);
			}
		} //RowLayout

		RowLayout {
			enabled: !setManager.completed
			Layout.fillWidth: true

			SetInputField {
				id: txtNWeight
				type: SetInputField.Type.WeightType
				text: setManager.weight1
				availableWidth: btnCopySetWeight.visible ? controlWidth - 40 : controlWidth
				alternativeLabels: myoLabels

				onValueChanged: (str) => setManager.weight1 = str;
			}

			TPButton {
				id: btnCopySetWeight
				visible: false
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: exerciseManager.copyWeightValueIntoOtherSets(setManager.number);
			}
		} //RowLayout

		SetNotesField {
			id: btnShowHideNotes
			text: setManager.notes
			enabled: !setManager.completed
			Layout.fillWidth: true

			onEditFinished: (new_text) => setManager.notes = new_text;
		}

		TPButton {
			id: btnCompleteExercise
			text: qsTr("Exercise completed")
			flat: false
			visible: setManager.lastSet
			enabled: setManager.finishButtonEnabled
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				setLayout.enabled = false;
				exerciseManager.exerciseCompleted();
			}
		}
	} // setLayout
} // Item
