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

	required property ExerciseManager exerciseManager
	required property DBTrainingDayModel tDayModel

	property bool bCanEditRestTimeTracking
	property bool bTrackRestTime
	property bool bAutoRestTime
	property bool bListRequestForExercise1: false
	property bool bListRequestForExercise2: false
	property bool bCompositeExercise

	signal requestSimpleExercisesList(Item requester, var bVisible, var bMultipleSelection, int id)
	signal requestFloatingButton(var exerciseidx, var settype, var nset)
	signal showRemoveExerciseMessage(int exerciseidx)

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
			visible: tDayModel.dayIsEditable
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
			enabled: entryManager.exerciseIdx < tDayModel.exerciseCount-1
			visible: tDayModel.dayIsEditable
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
					text: tDayModel.exerciseName(entryManager.exerciseIdx)
					bEditable: tDayModel.dayIsEditable
					width: appSettings.pageWidth - 55
					height: 70
					Layout.minimumWidth: width
					Layout.maximumWidth: width
					Layout.leftMargin: 0

					Keys.onReturnPressed: txtNReps.forceActiveFocus();
					onExerciseChanged: (new_text) => {
						tDayModel.setExerciseName(new_text, entryManager.exerciseIdx);
						exerciseManager.changeSetsExerciseLabels(exerciseIdx, 1, tDayModel.exerciseName1(exerciseIdx), false);
						exerciseManager.changeSetsExerciseLabels(exerciseIdx, 2, tDayModel.exerciseName2(exerciseIdx), false);
					}
					onRemoveButtonClicked: showRemoveExerciseMessage(entryManager.exerciseIdx);
					onEditButtonClicked: requestSimpleExercisesList(exerciseItem, !readOnly, true, 1);
					onItemClicked: paneExerciseShowHide(false, false);
				}
			} //Row txtExerciseName

			RowLayout {
				id: trackRestTimeRow
				enabled: tDayModel.dayIsEditable && bCanEditRestTimeTracking
				Layout.fillWidth: true
				Layout.leftMargin: 5

				TPCheckBox {
					id: chkTrackRestTime
					text: qsTr("Track rest times?")
					textColor: "black"
					width: paneExercise.width/2 - 10

					Component.onCompleted: checked = bTrackRestTime;

					onClicked: exerciseManager.manageRestTime(entryManager.exerciseIdx, checked, bAutoRestTime, cboSetType.currentIndex);
				}

				TPCheckBox {
					id: chkAutoRestTime
					text: qsTr("Auto tracking")
					textColor: "black"
					width: paneExercise.width/2 - 10
					enabled: bTrackRestTime
					checked: bAutoRestTime

					onPressAndHold: ToolTip.show(qsTr("Tap on Start Rest/Stop Rest to have the rest time automatically recorded"), 5000);
					onClicked: {
						bAutoRestTime = checked;
						exerciseManager.manageRestTime(entryManager.exerciseIdx, bTrackRestTime, bAutoRestTime, cboSetType.currentIndex);
					}
				}
			}

			RowLayout {
				enabled: tDayModel.dayIsEditable
				Layout.topMargin: 10
				Layout.leftMargin: 5

				SetInputField {
					id: txtNReps
					text: exerciseManager.exerciseReps(exerciseIdx, 0)
					type: SetInputField.Type.RepType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str) => exerciseManager.setExerciseReps(exerciseIdx, 0, str);
					onEnterOrReturnKeyPressed: !txtNReps2.visible ? txtNWeight.forceActiveFocus() : txtNReps2.forceActiveFocus();
				}

				SetInputField {
					id: txtNWeight
					text: exerciseManager.exerciseWeight(exerciseIdx, 0)
					type: SetInputField.Type.WeightType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str) => exerciseManager.setExerciseWeight(exerciseIdx, 0, str);
				}
			}

			RowLayout {
				enabled: tDayModel.dayIsEditable
				visible: bCompositeExercise
				Layout.leftMargin: 5

				SetInputField {
					id: txtNReps2
					text: exerciseManager.exerciseReps(exerciseIdx, 1)
					type: SetInputField.Type.RepType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str) => exerciseManager.setExerciseReps(exerciseIdx, 1, str)
					onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
				}

				SetInputField {
					id: txtNWeight2
					text: exerciseManager.exerciseWeight(exerciseIdx, 1)
					type: SetInputField.Type.WeightType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onVisibleChanged: cboSetType.currentIndex = visible ? 4 : 0
					onValueChanged: (str) => exerciseManager.setExerciseWeight(exerciseIdx, 1, str);
				}
			}

			SetInputField {
				id: txtRestTime
				type: SetInputField.Type.TimeType
				text: tDayModel.nextSetSuggestedTime(exerciseIdx, cboSetType.currentIndex, 0);
				availableWidth: paneExercise.width/2
				backColor: "transparent"
				borderColor: "transparent"
				enabled: bTrackRestTime && !bAutoRestTime
				Layout.leftMargin: 5

				onValueChanged: (str) => nRestTime = str;
			}

			Label {
				text: qsTr("Set type: ")
				font.bold: true
				Layout.leftMargin: 5
			}

			RowLayout {
				enabled: tDayModel.dayIsEditable
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				Layout.bottomMargin: 10
				spacing: 1

				TPComboBox {
					id: cboSetType
					currentIndex: exerciseManager.exerciseDefaultSetType(exerciseIdx);
					model: AppGlobals.setTypesModel
					implicitWidth: 160

					onActivated: (index) => exerciseManager.setExerciseDefaultSetType(exerciseIdx, index);
				}

				SetInputField {
					id: txtNSets
					text: exerciseManager.exerciseSets(exerciseIdx)
					type: SetInputField.Type.SetType
					availableWidth: layoutMain.width / 3
					alternativeLabels: ["","","",qsTr("sets #:")]
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str)=> nSets = str;
				}

				TPButton {
					id: btnAddSet
					imageSource: "add-new"
					imageSize: 30
					Layout.leftMargin: 15

					onClicked: {
						exerciseManager.addNewSet(exercise_idx);
						if (cboSetType.currentIndex == 4)
							cboSetType.enabled = false;
						else
							cboSetType.enableIndex(4, false);
						requestFloatingButton(exerciseIdx, cboSetType.currentIndex, (exerciseManager.exerciseSetsCount(exercise_idx) + 1).toString());
					}
				}
			} // RowLayout

			ColumnLayout {
				id: exerciseSetsLayout
				objectName: "exerciseSetsLayout"
				spacing: 0
				width: appSettings.pageWidth - 10
				Layout.fillWidth: true
				Layout.fillHeight: true
			}
		} // ColumnLayout layoutMain
	} //paneExercise

	function changeExercise(fromList: bool) {
		var interruptSignals = true;
		if (bListRequestForExercise1) {
			if (fromList)
				exerciseManager.changeSetsExerciseLabels(exerciseIdx, 1, exercisesModel.selectedEntriesValue(0, 1) + " - " + exercisesModel.selectedEntriesValue(0, 2));
			else
				exerciseManager.changeSetsExerciseLabels(exerciseIdx, 1, tDayModel.exerciseName1(exerciseIdx), false);
			bListRequestForExercise1 = false;
		}
		else if (bListRequestForExercise2) {
			if (fromList)
				exerciseManager.changeSetsExerciseLabels(exerciseIdx, 2, exercisesModel.selectedEntriesValue(0, 1) + " - " + exercisesModel.selectedEntriesValue(0, 2));
			else
				exerciseManager.changeSetsExerciseLabels(exerciseIdx, 2, tDayModel.exerciseName2(exerciseIdx), false);
			bListRequestForExercise2 = false;
		}
		else
		{
			interruptSignals = false;
			if (fromList)
				tDayModel.changeExerciseName(exerciseIdx, exercisesModel);
			else
			{
				if (bListRequestForExercise1)
					exerciseManager.changeSetsExerciseLabels(exerciseIdx, 1, tDayModel.exerciseName1(exerciseIdx), false);
				else if (bListRequestForExercise2)
					exerciseManager.changeSetsExerciseLabels(exerciseIdx, 2, tDayModel.exerciseName2(exerciseIdx), false);
			}
		}

		if (interruptSignals)
		{
			txtExerciseName.bCanEmitTextChanged = false;
			txtExerciseName.text = tDayModel.exerciseName(exerciseIdx);
			txtExerciseName.bCanEmitTextChanged = true;
		}
		else
			txtExerciseName.text = tDayModel.exerciseName(exerciseIdx);
	}

	function changeExercise1(showList: bool) {
		bListRequestForExercise1 = true;
		requestSimpleExercisesList(exerciseItem, showList, false, 1);
	}

	function changeExercise2(showList: bool) {
		bListRequestForExercise2 = true;
		requestSimpleExercisesList(exerciseItem, showList, false, 1);
	}

	function moveExercise(up: bool, cxx_cal: bool) {
		if (cxx_cal)
			exerciseManager.moveExercise(exerciseIdx, up ? --exerciseIdx : ++exerciseIdx);
		else {
			if (up) --exerciseIdx;
			else ++exerciseIdx;
		}

		lblExerciseNumber.text = parseInt(exerciseIdx + 1) + ":";
		txtExerciseName.text = tDayModel.exerciseName(exerciseIdx);
		exerciseItem.Layout.row = exerciseIdx;
	}

	function paneExerciseShowHide(show: bool, force: bool) {
		paneExercise.shown = force ? show : !paneExercise.shown
		if (paneExercise.shown)
			exerciseManager.getSetObjects(exerciseIdx);
	}

	function liberateSignals(liberate: bool) {
		txtExerciseName.bCanEmitTextChanged = liberate;
	}
} //Item
