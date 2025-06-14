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

	readonly property int controlWidth: setItem.width - 20

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
					exerciseManager.updateSetTypeForNextSets(setManager.number);
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

		RowLayout {
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

				onClicked: exerciseManager.updateRestTimeForNextSets(setManager.number);
			}
		}

		RowLayout {
			enabled: !setManager.completed
			uniformCellSizes: true
			spacing: 30
			Layout.alignment: Qt.AlignCenter
			Layout.fillWidth: true
			Layout.topMargin: 10
			Layout.bottomMargin: 10

			TPButton {
				id: btnAddSubSet
				imageSource: "add-new"
				enabled: setManager.nSubSets <= 3
				Layout.preferredWidth: imageSize
				Layout.preferredHeight: imageSize

				onClicked: setManager.addSubSet();
			}

			TPButton {
				id: btnDelSubSet
				imageSource: "remove"
				enabled: setManager.nSubSets > 1
				Layout.preferredWidth: imageSize
				Layout.preferredHeight: imageSize

				onClicked: setManager.removeSubSet();
			}
		}

		Repeater {
			id: subSetsRepeater
			model: setManager.nSubSets
			enabled: !setManager.completed
			Layout.fillWidth: true

			RowLayout {
				id: subSetsLayout
				Layout.fillWidth: true

				SetInputField {
					id: txtNReps
					type: SetInputField.Type.RepType
					availableWidth: controlWidth*0.6
					showLabel: !btnCopySetReps.visible

					onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();

					Component.onCompleted: {
						text = Qt.binding(function() {
							switch(index) {
								case 0: return setManager.reps1;
								case 1: return setManager.reps2;
								case 2: return setManager.reps3;
								case 3: return setManager.reps4;
							}
						});
						valueChanged.connect(function(str) {
							switch(index) {
								case 0: setManager.reps1 = str; break;
								case 1: setManager.reps2 = str; break;
								case 2: setManager.reps3 = str; break;
								case 3: setManager.reps4 = str; break;
							}
						});
					}
				}

				TPButton {
					id: btnCopySetReps
					visible: false
					imageSource: "copy-setvalue"
					height: 25
					width: 25
					Layout.leftMargin: 10

					Component.onCompleted: {
						switch (index) {
							case 0:
								setManager.reps1Changed.connect(function () { visible = setManager.isManuallyModified; });
								setManager.isManuallyModifiedChanged.connect(function() { if (!setManager.isManuallyModified) visible = false; })
							break;
							case 1:
								setManager.reps2Changed.connect(function () { visible = setManager.isManuallyModified; });
								setManager.isManuallyModifiedChanged.connect(function() { if (!setManager.isManuallyModified) visible = false; })
							break;
							case 2:
								setManager.reps3Changed.connect(function () { visible = setManager.isManuallyModified; });
								setManager.isManuallyModifiedChanged.connect(function() { if (!setManager.isManuallyModified) visible = false; })
							break;
							case 3:
								setManager.reps4Changed.connect(function () { visible = setManager.isManuallyModified; });
								setManager.isManuallyModifiedChanged.connect(function() { if (!setManager.isManuallyModified) visible = false; })
							break;
						}
					}

					onClicked: exerciseManager.updateRepsForNextSets(setManager.number, index);
				}

				SetInputField {
					id: txtNWeight
					type: SetInputField.Type.WeightType
					availableWidth: controlWidth*0.4
					showLabel: false

					Component.onCompleted: {
						text = Qt.binding(function() {
							switch(index) {
								case 0: return setManager.weight1;
								case 1: return setManager.weight2;
								case 2: return setManager.weight3;
								case 3: return setManager.weight4;
							}
						});
						txtNWeight.valueChanged.connect(function(str) {
							switch(index) {
								case 0: setManager.weight1 = str; break;
								case 1: setManager.weight2 = str; break;
								case 2: setManager.weight3 = str; break;
								case 3: setManager.weight4 = str; break;
							}
						});
					}
				}

				TPButton {
					id: btnCopySetWeight
					visible: false
					imageSource: "copy-setvalue"
					height: 25
					width: 25
					Layout.leftMargin: 10

					Component.onCompleted: {
						switch (index) {
							case 0:
								setManager.weight1Changed.connect(function () { visible = setManager.isManuallyModified; });
								setManager.isManuallyModifiedChanged.connect(function() { if (!setManager.isManuallyModified) visible = false; })
							break;
							case 1:
								setManager.weight2Changed.connect(function () { visible = setManager.isManuallyModified; });
								setManager.isManuallyModifiedChanged.connect(function() { if (!setManager.isManuallyModified) visible = false; })
							break;
							case 2:
								setManager.weight3Changed.connect(function () { visible = setManager.isManuallyModified; });
								setManager.isManuallyModifiedChanged.connect(function() { if (!setManager.isManuallyModified) visible = false; })
							break;
							case 3:
								setManager.weight4Changed.connect(function () { visible = setManager.isManuallyModified; });
								setManager.isManuallyModifiedChanged.connect(function() { if (!setManager.isManuallyModified) visible = false; })
							break;
						}
					}

					onClicked: exerciseManager.updateWeightForNextSets(setManager.number, index);
				}
			}
		}

		SetNotesField {
			id: btnShowHideNotes
			text: setManager.notes
			enabled: !setManager.completed
			Layout.fillWidth: true

			onEditFinished: (new_text) => workoutModel.setSetNotes(setNumber, exerciseNumber, new_text);
		}
	} // setLayout

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
