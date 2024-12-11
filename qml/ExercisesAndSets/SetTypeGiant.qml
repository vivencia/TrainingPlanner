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
	Layout.topMargin: 5
	Layout.bottomMargin: 5

	required property SetEntryManager setManager
	required property ExerciseEntryManager exerciseManager

	readonly property int controlWidth: setItem.width - 20

	Connections {
		target: setManager
			function onTypeChanged() { btnCopySetType.visible = !btnCopySetType.visible; }
			function onRestTimeChanged() { btnCopyTimeValue.visible = !btnCopyTimeValue.visible; }
			function onReps1Changed() { btnCopySetReps1.visible = !btnCopySetReps1.visible; }
			function onWeight1Changed() { btnCopySetWeight1.visible = !btnCopySetWeight1.visible; }
			function onReps1Changed() { btnCopySetReps2.visible = !btnCopySetReps2.visible; }
			function onWeight1Changed() { btnCopySetWeight2.visible = !btnCopySetWeight2.visible; }
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
					left: btnCopySetType.visible ? btnCopySetType.right : cbosetManager.type.right
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

		RowLayout {
			uniformCellSizes: true
			enabled: !setCompleted
			spacing: 0
			Layout.fillWidth: true

			TPLabel {
				id: lblExercise1
				text: setManager.exerciseName1
				wrapMode: Text.WordWrap
				fontColor: "black"
				width: controlWidth*0.5
				Layout.preferredWidth: width
				Layout.preferredHeight: _preferredHeight
				Layout.alignment: Qt.AlignHCenter

				MouseArea {
					anchors.fill: parent
					onClicked: exerciseManager.simpleExercisesList(true, false, 1);
				}
			}

			TPLabel {
				id: lblExercise2
				text: setManager.exerciseName2
				wrapMode: Text.WordWrap
				fontColor: "black"
				width: controlWidth*0.5
				Layout.preferredWidth: width
				Layout.preferredHeight: _preferredHeight
				Layout.alignment: Qt.AlignHCenter

				MouseArea {
					anchors.fill: parent
					onClicked: exerciseManager.simpleExercisesList(true, false, 2);
				}
			}
		}

		Row {
			enabled: !setManager.completed
			Layout.fillWidth: true

			SetInputField {
				id: txtNReps1
				type: SetInputField.Type.RepType
				text: setManager.reps1
				availableWidth: btnCopySetReps1.visible ? controlWidth - 40 : controlWidth
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

				onClicked: exerciseManager.copyRepsValueIntoOtherSets(setManager.number, 0);
			}

			SetInputField {
				id: txtNReps2
				type: SetInputField.Type.RepType
				text: setManager.reps2
				availableWidth: controlWidth*0.3
				showLabel: false

				onValueChanged: (str) => setManager.reps2 = str;
				onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
			}

			TPButton {
				id: btnCopySetReps2
				imageSource: "copy-setvalue"
				height: 25
				width: 25

				onClicked: exerciseManager.copyRepsValueIntoOtherSets(setManager.number, 1);
			}
		}

		RowLayout {
			enabled: !setManager.completed
			Layout.fillWidth: true

			SetInputField {
				id: txtNWeight1
				text: setManager.weight1
				type: SetInputField.Type.WeightType
				availableWidth: btnCopySetWeight1.visible ? controlWidth - 40 : controlWidth
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

				onClicked: exerciseManager.copyWeightValueIntoOtherSets(setManager.number, 0);
			}

			SetInputField {
				id: txtNWeight2
				type: SetInputField.Type.WeightType
				text: setManager.weight2
				availableWidth: controlWidth*0.3
				showLabel: false

				onValueChanged: (str) => setManager.weight2 = str;
			}

			TPButton {
				id: btnCopySetWeight2
				imageSource: "copy-setvalue"
				height: 25
				width: 25

				onClicked: exerciseManager.copyWeightValueIntoOtherSets(setManager.number, 1);
			}
		}

		SetNotesField {
			id: btnShowHideNotes
			text: setManager.notes
			enabled: !setManager.completed
			Layout.fillWidth: true

			onEditFinished: (new_text) => tDayModel.setSetNotes(setNumber, exerciseIdx, new_text);
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
	} //ColumnLayout setLayout
} // Item
