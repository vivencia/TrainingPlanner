import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

FocusScope {
	id: setItem
	implicitHeight: setLayout.childrenRect.height*1.25 + setModeItem.height
	Layout.fillWidth: true

	required property SetEntryManager setManager
	required property ExerciseEntryManager exerciseManager

	readonly property int controlWidth: setItem.width - 20
	readonly property var myoLabels: setManager.type === 5 ? [ qsTr("Weight:"), setManager.number === 0 ? qsTr("Reps to failure:") : qsTr("Reps to match:"),
						qsTr("Rest time:"), qsTr("Number of short rest pauses:") ] : []

	Connections {
		target: setManager
			function onTypeChanged() { btnCopySetType.visible = setManager.isManuallyModified; }
			function onRestTimeChanged() { btnCopyTimeValue.visible = setManager.isManuallyModified; }
			function onReps1Changed() { btnCopySetReps.visible = setManager.isManuallyModified; }
			function onWeight1Changed() { btnCopySetWeight.visible = setManager.isManuallyModified; }
			function onIsManuallyModifiedChanged() {
					if (!setManager.isManuallyModified)
						btnCopySetType.visible = btnCopyTimeValue.visible = btnCopySetReps.visible = btnCopySetWeight.visible = false;
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
			enabled: setManager.current
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
		}

		Label {
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
				Layout.leftMargin: 10

				onClicked: {
					exerciseManager.copyTimeValueIntoOtherSets(setManager.number);
					visible = false;
				}
			}
		}

		SetInputField {
			id: txtNSubSets
			type: SetInputField.SetType
			text: setManager.subSets
			availableWidth: controlWidth
			visible: setManager.hasSubSets
			labelText: myoLabels
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
				labelText: myoLabels

				onValueChanged: (str) => setManager.reps1 = str;
				onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
			}

			TPButton {
				id: btnCopySetReps
				visible: false
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.leftMargin: 10

				onClicked: {
					exerciseManager.copyRepsValueIntoOtherSets(setManager.number);
					visible = false;
				}
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
				labelText: myoLabels

				onValueChanged: (str) => setManager.weight1 = str;
			}

			TPButton {
				id: btnCopySetWeight
				visible: false
				imageSource: "copy-setvalue"
				height: 25
				width: 25
				Layout.leftMargin: 10

				onClicked: {
					exerciseManager.copyWeightValueIntoOtherSets(setManager.number);
					visible = false;
				}
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
			Layout.topMargin: -10
			Layout.preferredWidth: implicitWidth

			onClicked: {
				setLayout.enabled = false;
				exerciseManager.exerciseCompleted();
			}
		}
	} // setLayout
} // Item
