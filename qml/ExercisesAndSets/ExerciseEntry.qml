import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

FocusScope {
	id: exerciseItem
	Layout.fillWidth: true
	implicitHeight: paneExercise.height

	required property ExerciseEntryManager exerciseManager

	property bool bListRequestForExercise1: false
	property bool bListRequestForExercise2: false

	signal requestSimpleExercisesList(Item requester, var bVisible, var bMultipleSelection, int id)
	signal requestFloatingButton(var exerciseidx, var settype, var nset)

	Frame {
		id: paneExercise
		property bool shown: entryManager.nSets === 0
		visible: height > 0
		height: shown ? implicitHeight : txtExerciseName.height + 20
		implicitHeight: layoutMain.implicitHeight + 10
		implicitWidth: width
		width: appSettings.pageWidth - 10
		clip: true
		padding: 0
		spacing: 0
		z: 0
		Layout.fillWidth: true

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		background: Rectangle {
			color: entryManager.exerciseIdx % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
			border.color: "transparent"
			opacity: 0.8
			radius: 10
		}

		TPButton {
			id: btnMoveExerciseUp
			imageSource: "up"
			hasDropShadow: false
			enabled: entryManager.exerciseIdx > 0
			visible: exerciseManager.isEditable
			height: 30
			width: 30

			anchors {
				left: parent.left
				leftMargin: 0
				top: parent.top
				topMargin: -15
			}

			onClicked: moveExercise(true, true);
		}

		TPButton {
			id: btnMoveExerciseDown
			imageSource: "down"
			hasDropShadow: false
			enabled: !exerciseManager.lastExercise
			visible: exerciseManager.isEditable
			height: 30
			width: 30

			anchors {
				left: parent.left
				leftMargin: 20
				top: parent.top
				topMargin: -15
			}

			onClicked: moveExercise(false, true);
		}

		ColumnLayout {
			id: layoutMain
			anchors.fill: parent
			spacing: 0

			RowLayout {
				spacing: 0
				Layout.fillWidth: true
				Layout.leftMargin: -10
				Layout.topMargin: 5

				TPButton {
					id: btnFoldIcon
					imageSource: paneExercise.shown ? "black/fold-up" : "black/fold-down"
					hasDropShadow: false
					imageSize: 18
					onClicked: paneExerciseShowHide(false, false);
					Layout.leftMargin: 8
					z: 1
				}

				Label {
					id: lblExerciseNumber
					text: entryManager.exerciseNumber() + ":"
					font.bold: true
					font.pointSize: appSettings.fontSizeText
					width: 15
					Layout.leftMargin: 0
				}

				ExerciseNameField {
					id: txtExerciseName
					text: exerciseManager.exerciseName
					bEditable: exerciseManager.isEditable
					width: appSettings.pageWidth - 55
					height: 70
					Layout.minimumWidth: width
					Layout.maximumWidth: width
					Layout.leftMargin: 0

					Keys.onReturnPressed: txtNReps.forceActiveFocus();
					onExerciseChanged: (new_text) => exerciseManager.exerciseName = new_text;
					onRemoveButtonClicked: exerciseManager.removeExercise();
					onEditButtonClicked: requestSimpleExercisesList(exerciseItem, !readOnly, true, 1);
					onItemClicked: paneExerciseShowHide(false, false);
				}
			} //Row txtExerciseName

			RowLayout {
				id: trackRestTimeRow
				enabled: exerciseManager.isEditable && exerciseManager.canEditRestTimeTracking
				Layout.fillWidth: true
				Layout.leftMargin: 5

				TPCheckBox {
					id: chkTrackRestTime
					text: qsTr("Track rest times?")
					textColor: "black"
					checked: exerciseManager.trackRestTime
					width: paneExercise.width/2 - 10

					onClicked: exerciseManager.trackRestTime = checked;
				}

				TPCheckBox {
					id: chkAutoRestTime
					text: qsTr("Auto tracking")
					textColor: "black"
					width: paneExercise.width/2 - 10
					enabled: exerciseManager.trackRestTime
					checked: exerciseManager.autoRestTime

					onPressAndHold: ToolTip.show(qsTr("Tap on Start Rest/Stop Rest to have the rest time automatically recorded"), 5000);
					onClicked: exerciseManager.autoRestTime = checked;
				}
			}

			RowLayout {
				enabled: exerciseManager.isEditable
				Layout.topMargin: 10
				Layout.leftMargin: 5

				SetInputField {
					id: txtNReps
					text: exerciseManager.repsForExercise1
					type: SetInputField.Type.RepType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str) => exerciseManager.repsForExercise1 = str;
					onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
				}

				SetInputField {
					id: txtNWeight
					text: exerciseManager.weightForExercise1
					type: SetInputField.Type.WeightType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str) => exerciseManager.weightForExercise1 = str;
					onEnterOrReturnKeyPressed: !exerciseManager.compositeExercise ? txtNSets.forceActiveFocus() : txtNReps2.forceActiveFocus();
				}
			}

			RowLayout {
				enabled: exerciseManager.isEditable
				visible: exerciseManager.compositeExercise
				Layout.leftMargin: 5

				SetInputField {
					id: txtNReps2
					text: exerciseManager.repsForExercise2
					type: SetInputField.Type.RepType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str) => exerciseManager.repsForExercise2 = str;
					onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
				}

				SetInputField {
					id: txtNWeight2
					text: exerciseManager.weightForExercise2
					type: SetInputField.Type.WeightType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str) => exerciseManager.weightForExercise2 = str;
					onEnterOrReturnKeyPressed: txtNSets.forceActiveFocus();
				}
			}

			SetInputField {
				id: txtRestTime
				type: SetInputField.Type.TimeType
				text: exerciseManager.restTime
				availableWidth: paneExercise.width/2
				backColor: "transparent"
				borderColor: "transparent"
				enabled: exerciseManager.trackRestTime && !exerciseManager.autoRestTime
				Layout.leftMargin: 5

				onValueChanged: (str) => exerciseManager.restTime = str;
			}

			Label {
				text: qsTr("Set type: ")
				font.bold: true
				Layout.leftMargin: 5
			}

			RowLayout {
				enabled: exerciseManager.isEditable
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				Layout.bottomMargin: 10
				spacing: 1

				TPComboBox {
					id: cboSetType
					currentIndex: exerciseManager.newSetType
					model: AppGlobals.setTypesModel
					implicitWidth: 160

					onActivated: (index) => exerciseManager.newSetType = index;
				}

				SetInputField {
					id: txtNSets
					text: exerciseManager.setsNumber
					type: SetInputField.Type.SetType
					availableWidth: layoutMain.width / 3
					alternativeLabels: ["","","",qsTr("sets #:")]
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str)=> exerciseManager.setsNumber = str;
				}

				TPButton {
					id: btnAddSet
					imageSource: "add-new"
					imageSize: 30
					Layout.leftMargin: 15

					onClicked: {
						exerciseManager.addNewSet();
						if (cboSetType.currentIndex === 4)
							cboSetType.enabled = false;
						else
							cboSetType.enableIndex(4, false);
						requestFloatingButton(exerciseIdx, cboSetType.currentIndex, (exerciseManager.exerciseSetsCount(exercise_idx) + 1).toString());
					}
				}
			} // RowLayout
		} // ColumnLayout layoutMain

		GridLayout {
			id: exerciseSetsLayout
			objectName: "exerciseSetsLayout"
			width: appSettings.pageWidth - 10
			columns: 1

			anchors {
				left: parent.left
				leftMargin: 5
				rightMargin: 5
				right:parent.right
				top: layoutMain.bottom
			}
		}
	} //paneExercise

	function paneExerciseShowHide(show: bool, force: bool) {
		paneExercise.shown = force ? show : !paneExercise.shown
		if (paneExercise.shown)
			exerciseManager.createAvailableSets();
	}
} //Item
