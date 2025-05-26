import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

FocusScope {
	id: setItem
	implicitHeight: setLayout.childrenRect.height + setModeItem.height + btnCompleteExercise.height + 20
	Layout.fillWidth: true

	required property SetEntryManager setManager
	required property ExerciseEntryManager exerciseManager

	readonly property int controlWidth: (setItem.width - 20)/2
	readonly property list<string> myoLabels: [ qsTr("Weight:"), setManager.number === 0 ? qsTr("Reps to failure:") : qsTr("Reps to match:"),
						qsTr("Rest time:"), qsTr("Number of short rest pauses:") ]


	Connections {
		target: setManager
			function onTypeChanged() { btnCopySetType.visible = setManager.isManuallyModified; }
			function onRestTimeChanged() { btnCopyTimeValue.visible = setManager.isManuallyModified; }
			function onReps1Changed() { btnCopySetReps1.visible = setManager.isManuallyModified; }
			function onWeight1Changed() { btnCopySetWeight1.visible = setManager.isManuallyModified; }
			function onReps2Changed() { btnCopySetReps2.visible = setManager.isManuallyModified; }
			function onWeight2Changed() { btnCopySetWeight2.visible = setManager.isManuallyModified; }

			function onIsManuallyModifiedChanged() {
					if (!setManager.isManuallyModified) {
						btnCopySetType.visible = btnCopyTimeValue.visible = btnCopySetReps1.visible = btnCopySetWeight1.visible =
							btnCopySetReps2.visible = btnCopySetWeight2.visible = false;
					}
			}
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

	Item {
		id: setModeItem
		enabled: setManager.isEditable
		height: 30
		width: parent.width

		anchors {
			top: parent.top
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}

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

	ColumnLayout {
		id: setLayout
		enabled: setManager.isEditable ? setManager.current : false

		anchors {
			top: setModeItem.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: btnCompleteExercise.top
			bottomMargin: 5
		}

		TPLabel {
			id: lblSetNumber
			text: qsTr("Set #") + setManager.strNumber
			font.bold: true
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
					left: cboSetType.right
					leftMargin: 10
				}

				onClicked: {
					exerciseManager.copyTypeValueIntoOtherSets(setManager.number);
					visible = false;
				}
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

		Row {
			enabled: !setManager.completed
			spacing: 10
			Layout.fillWidth: true

			TPButton {
				id: lblExercise1
				text: setManager.exerciseName1
				width: controlWidth
				autoResize: true
				rounded: false

				onClicked: exerciseManager.simpleExercisesList(true, false, 1);
			}

			TPButton {
				id: lblExercise2
				text: setManager.exerciseName2
				width: controlWidth
				autoResize: true
				rounded: false

				onClicked: exerciseManager.simpleExercisesList(true, false, 2);
			}
		}

		Row {
			enabled: !setManager.completed
			spacing: 5
			Layout.fillWidth: true

			SetInputField {
				id: txtNReps1
				type: SetInputField.Type.RepType
				text: setManager.reps1
				availableWidth: btnCopySetReps1.visible ? controlWidth*1.3 - 30 : controlWidth*1.3
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

				onClicked: {
					exerciseManager.copyRepsValueIntoOtherSets(setManager.number, 0);
					visible = false;
				}
			}

			SetInputField {
				id: txtNReps2
				type: SetInputField.Type.RepType
				text: setManager.reps2
				availableWidth: controlWidth*0.6
				showLabel: false

				onValueChanged: (str) => setManager.reps2 = str;
				onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
			}

			TPButton {
				id: btnCopySetReps2
				imageSource: "copy-setvalue"
				visible: false
				height: 25
				width: 25

				onClicked: {
					exerciseManager.copyRepsValueIntoOtherSets(setManager.number, 1);
					visible = false;
				}
			}
		}

		RowLayout {
			enabled: !setManager.completed
			spacing: 5
			Layout.fillWidth: true

			SetInputField {
				id: txtNWeight1
				text: setManager.weight1
				type: SetInputField.Type.WeightType
				availableWidth: btnCopySetWeight1.visible ? controlWidth*1.3 - 40 : controlWidth*1.3
				showLabel: !btnCopySetWeight1.visible

				onValueChanged: (str) => setManager.weight1 = str;
				onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();
			}

			TPButton {
				id: btnCopySetWeight1
				imageSource: "copy-setvalue"
				visible: false
				height: 25
				width: 25

				onClicked: {
					exerciseManager.copyWeightValueIntoOtherSets(setManager.number, 0);
					visible = false;
				}
			}

			SetInputField {
				id: txtNWeight2
				type: SetInputField.Type.WeightType
				text: setManager.weight2
				availableWidth: controlWidth*0.6
				showLabel: false

				onValueChanged: (str) => setManager.weight2 = str;
			}

			TPButton {
				id: btnCopySetWeight2
				imageSource: "copy-setvalue"
				visible: false
				height: 25
				width: 25

				onClicked: {
					exerciseManager.copyWeightValueIntoOtherSets(setManager.number, 1);
					visible = false;
				}
			}
		}

		SetNotesField {
			id: btnShowHideNotes
			text: setManager.notes
			enabled: !setManager.completed
			Layout.fillWidth: true

			onEditFinished: (new_text) => tDayModel.setSetNotes(setNumber, exerciseNumber, new_text);
		}
	} //ColumnLayout setLayout

	TPButton {
		id: btnCompleteExercise
		text: qsTr("Exercise completed")
		flat: false
		visible: setManager.lastSet && exerciseManager.isEditable
		enabled: exerciseManager.allSetsCompleted
		height: visible ? 30 : 0

		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
			bottomMargin: 5
		}

		onClicked: {
			setLayout.enabled = false;
			exerciseManager.exerciseCompleted();
		}
	}
} // Item
