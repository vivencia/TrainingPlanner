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

			TPButton {
				id: btnAddSubSet
				text: qsTr("Add subset")
				imageSource: "add-new.png"
				flat: false
				rounded: false
				enabled: setManager.nSubSets <= 3
				width: controlWidth/2 + 10
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				onClicked: setManager.nSubSets = setManager.nSubSets + 1;
			}

			TPButton {
				id: btnDelSubSet
				text: qsTr("Remove subset")
				imageSource: "remove.png"
				flat: false
				rounded: false
				enabled: setManager.nSubSets > 1
				width: controlWidth/2 + 10
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				onClicked: setManager.nSubSets = setManager.nSubSets - 1;
			}
		}

		Repeater {
			id: subSetsRepeater
			model: setManager.nSubSets
			enabled: !setManager.completed
			Layout.fillWidth: true

			RowLayout {
				id: subSetsLayout
				anchors.fill: parent

				SetInputField {
					id: txtNReps
					type: SetInputField.Type.RepType
					availableWidth: controlWidth/3
					Layout.alignment: Qt.AlignLeft
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
					Layout.alignment: Qt.AlignRight

					Component.onCompleted: {
						switch (index) {
							case 0: setManager.reps1Changed.connect(function () { visible = !visible; } ); break;
							case 1: setManager.reps2Changed.connect(function () { visible = !visible; } ); break;
							case 2: setManager.reps3Changed.connect(function () { visible = !visible; } ); break;
							case 3: setManager.reps4Changed.connect(function () { visible = !visible; } ); break;
						}
					}

					onClicked: exerciseManager.copyRepsValueIntoOtherSets(setManager.number, index);
				}

				SetInputField {
					id: txtNWeight
					type: SetInputField.Type.WeightType
					availableWidth: controlWidth/3
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
					Layout.alignment: Qt.AlignRight

					Component.onCompleted: {
						switch (index) {
							case 0: setManager.weight1Changed.connect(function () { visible = !visible; } ); break;
							case 1: setManager.weight2Changed.connect(function () { visible = !visible; } ); break;
							case 2: setManager.weight3Changed.connect(function () { visible = !visible; } ); break;
							case 3: setManager.weight4Changed.connect(function () { visible = !visible; } ); break;
						}
					}

					onClicked: exerciseManager.copyWeightValueIntoOtherSets(setManager.number, index);
				}
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
	} // setLayout
} // Item
