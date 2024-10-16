import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Item {
	id: setItem
	height: setLayout.height + 15
	implicitHeight: setLayout.implicitHeight + 15
	enabled: setManager.isEditable
	Layout.fillWidth: true
	Layout.leftMargin: 5
	Layout.rightMargin: 5

	required property SetEntryManager setManager
	required property ExerciseEntryManager exerciseManager

	readonly property int controlWidth: setItem.width - 20

	Connections {
		setManager.onTypeChanged: btnCopySetType.visible = !btnCopySetType.visible;
		setManager.onRestTimeChanged: btnCopyTimeValue.visible = !btnCopyTimeValue.visible;
		setManager.onReps1Changed: btnCopySetReps1.visible = !btnCopySetReps1.visible
		setManager.onWeight1Changed: btnCopySetWeight1.visible = !btnCopySetWeight1.visible
		setManager.onReps2Changed: btnCopySetReps2.visible = !btnCopySetReps2.visible
		setManager.onWeight2Changed: btnCopySetWeight2.visible = !btnCopySetWeight2.visible
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
		visible: bCurrentSet
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
		Layout.fillWidth: true
		Layout.bottomMargin: 5

		Item {
			height: 30
			Layout.fillWidth: true
			Layout.topMargin: 10

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
			Layout.topMargin: 10
			Layout.bottomMargin: 10

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
					left: cbosetManager.type.right
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
					left: btnCopySetType.visible ? btnCopySetType.right : cbosetManager.type.right
					leftMargin: 10
				}

				onClicked: exerciseManager.removeSetObject(setManager.number);
			}
		}

		RowLayout {
			visible: setManager.number > 0 && setManager.trackRestTime
			enabled: !setManager.completed && !setManager.autoRestTime
			Layout.leftMargin: 5

			SetInputField {
				id: txtRestTime
				type: SetInputField.Type.TimeType
				text: setManager.restTime
				availableWidth: btnCopyTimeValue.visible ? controlWidth - 40 : controlWidth
				windowTitle: lblSetNumber.text
				showButtons: !setManager.autoRestTime

				onValueChanged: (str) => setManager.restTime = str;
				onEnterOrReturnKeyPressed: txtNReps1.forceActiveFocus();
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

		RowLayout {
			Layout.fillWidth: true
			enabled: !setCompleted

			ExerciseNameField {
				id: txtExercise1
				text: setManager.exerciseName1
				showRemoveButton: false
				width: controlWidth/2 + 10
				Layout.maximumWidth: width
				Layout.minimumWidth: width
				Layout.leftMargin: -10

				onExerciseChanged: (new_text) => setManager.exerciseName1 = new_text;
				onEditButtonClicked: exerciseManager.simpleExercisesList(true, false);
			}

			ExerciseNameField {
				id: txtExercise2
				text: setManager.exerciseName2
				showRemoveButton: false
				width: controlWidth/2 + 10
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: width
				Layout.minimumWidth: width
				Layout.leftMargin: -10

				onExerciseChanged: (new_text) => setManager.exerciseName2 = new_text;
				onEditButtonClicked: exerciseManager.simpleExercisesList(true, false);
			}
		}

		RowLayout {
			enabled: !setManager.completed
			Layout.fillWidth: true
			spacing: 5

			SetInputField {
				id: txtNReps1
				type: SetInputField.Type.RepType
				text: setManager.reps1
				availableWidth: btnCopySetReps1.visible ? controlWidth - 40 : controlWidth
				Layout.alignment: Qt.AlignLeft
				showLabel: !btnCopySetReps1.visible

				onValueChanged: (str) => setManager.reps1 = str;
				onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();
			}

			TPButton {
				id: btnCopySetReps1
				visible: false
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: exerciseManager.copyRepsValueIntoOtherSets(setManager.number);
			}

			SetInputField {
				id: txtNReps2
				type: SetInputField.Type.RepType
				text: setManager.reps2
				availableWidth: controlWidth/3
				showLabel: false

				onValueChanged: (str) => setManager.reps2 = str;
				onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
			}

			TPButton {
				id: btnCopySetReps2
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: exerciseManager.copyRepsValueIntoOtherSets(setManager.number, 1);
			}
		}

		RowLayout {
			enabled: !setManager.completed
			Layout.fillWidth: true
			spacing: 5

			SetInputField {
				id: txtNWeight1
				text: setManager.weight1
				type: SetInputField.Type.WeightType
				availableWidth: btnCopySetWeight1.visible ? controlWidth - 40 : controlWidth
				Layout.alignment: Qt.AlignLeft
				showLabel: !btnCopySetWeight1.visible

				onValueChanged: (str) => setManager.weight1 = str;
				onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();
			}

			TPButton {
				id: btnCopySetWeight1
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: exerciseManager.copyWeightValueIntoOtherSets(setManager.number);
			}

			SetInputField {
				id: txtNWeight2
				type: SetInputField.Type.WeightType
				text: setManager.weight2
				availableWidth: controlWidth/3 + 10
				showLabel: false

				onValueChanged: (str) => setManager.weight2 = str;
			}

			TPButton {
				id: btnCopySetWeight2
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.alignment: Qt.AlignRight

				onClicked: exerciseManager.copyWeightValueIntoOtherSets(setManager.number, 1);
			}
		}

		SetNotesField {
			id: btnShowHideNotes
			text: setManager.notes
			enabled: !setManager.completed
			Layout.leftMargin: 5
			Layout.rightMargin: 5
			Layout.fillWidth: true

			onEditFinished: (new_text) => tDayModel.setSetNotes(setNumber, exerciseIdx, new_text);
		}

		TPButton {
			id: btnCompleteExercise
			text: qsTr("Exercise completed")
			visible: setManager.lastSet
			enabled: setManager.finishButtonEnabled
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				setLayout.enabled = false;
				exerciseManager.exerciseCompleted();
			}
		}
	} //ColumnLayout setLayout
} // Item
