import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../"
import "../TPWidgets"

import com.vivenciasoftware.qmlcomponents

FocusScope {
	id: exerciseItem
	Layout.fillWidth: true
	implicitHeight: paneExercise.height

	property DBTrainingDayModel tDayModel
	property int exerciseIdx
	property var trainingDayPage

	property int setNbr: 0
	property string nSets
	property string nReps
	property string nWeight
	property bool bListRequestForExercise1: false
	property bool bListRequestForExercise2: false
	property bool bCompositeExercise

	signal requestSimpleExercisesList(Item requester, var bVisible, var bMultipleSelection, int id)
	signal requestFloatingButton(var exerciseidx, var settype, var nset)
	signal showRemoveExerciseMessage(int exerciseidx)

	Frame {
		id: paneExercise
		property bool shown: tDayModel.setsNumber(exerciseIdx) === 0
		visible: height > 0
		height: shown ? implicitHeight : txtExerciseName.height + 30
		implicitHeight: layoutMain.implicitHeight + 10
		implicitWidth: parent.width
		width: windowWidth - 10
		clip: true
		padding: 0
		z: 0
		Layout.fillWidth: true

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		MouseArea {
			enabled: tDayModel.dayIsFinished
			z:2
			anchors.fill: parent
			onClicked: (mouse) => {
				if (mouse.y < txtExerciseName.height)
					btnFoldIcon.clicked();
			}
		}

		background: Rectangle {
			color: exerciseIdx % 2 === 0 ? listEntryColor1 : listEntryColor2
			border.color: "transparent"
			opacity: 0.8
			radius: 10
		}

		TPRoundButton {
			id: btnMoveExerciseUp
			height: 30
			width: 30
			padding: 5
			enabled: exerciseIdx > 0
			visible: !tDayModel.dayIsFinished
			imageName: "up.png"
			anchors {
				left: parent.left
				leftMargin: 0
				top: parent.top
				topMargin: -15
			}

			onClicked: moveExercise(true, true);
		}
		TPRoundButton {
			id: btnMoveExerciseDown
			height: 30
			width: 30
			padding: 5
			enabled: exerciseIdx < tDayModel.exerciseCount-1
			visible: !tDayModel.dayIsFinished
			imageName: "down.png"
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
			enabled: !tDayModel.dayIsFinished

			RowLayout {
				spacing: 0
				Layout.fillWidth: true
				Layout.topMargin: 5

				TPRoundButton {
					id: btnFoldIcon
					height: 25
					width: 25
					imageName: paneExercise.shown ? "fold-up.png" : "fold-down.png"
					onClicked: paneExerciseShowHide(false, false);
					z: 1
				}

				Label {
					id: lblExerciseNumber
					text: parseInt(exerciseIdx + 1) + ":"
					font.pointSize: AppSettings.fontSizeText
					width: 15
				}

				ExerciseNameField {
					id: txtExerciseName
					text: tDayModel.exerciseName(exerciseIdx)
					width: windowWidth - 45
					height: 70
					Layout.minimumWidth: width
					Layout.maximumWidth: width
					Layout.leftMargin: -10

					Keys.onReturnPressed: txtNReps.forceActiveFocus();
					onExerciseChanged: (new_text) => {
						tDayModel.setExerciseName(new_text, exerciseIdx);
						itemManager.changeSetsExerciseLabels(exerciseIdx, 1, tDayModel.exerciseName1(exerciseIdx), false);
						itemManager.changeSetsExerciseLabels(exerciseIdx, 2, tDayModel.exerciseName2(exerciseIdx), false);
					}
					onRemoveButtonClicked: showRemoveExerciseMessage(exerciseIdx);
					onEditButtonClicked: requestSimpleExercisesList(exerciseItem, !readOnly, true, 1);
					onItemClicked: paneExerciseShowHide(false, false);
				}
			} //Row txtExerciseName

			Row {
				Layout.topMargin: 10
				Layout.leftMargin: 5

				SetInputField {
					id: txtNReps
					text: runCmd.getCompositeValue(0, nReps)
					type: SetInputField.Type.RepType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged:(str)=> nReps = runCmd.setCompositeValue_QML(0, str, nReps);
					onEnterOrReturnKeyPressed: !txtNReps2.visible ? txtNWeight.forceActiveFocus() : txtNReps2.forceActiveFocus();
				}

				SetInputField {
					id: txtNReps2
					text: runCmd.getCompositeValue(1, nReps)
					type: SetInputField.Type.RepType
					showLabel: false
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"
					visible: bCompositeExercise

					onValueChanged:(str)=> nReps = runCmd.setCompositeValue_QML(1, str, nReps);
					onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
				}
			}

			Row {
				Layout.leftMargin: 5

				SetInputField {
					id: txtNWeight
					text: runCmd.getCompositeValue(0, nWeight)
					type: SetInputField.Type.WeightType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged:(str)=> nWeight = runCmd.setCompositeValue_QML(0, str, nWeight);
				}

				SetInputField {
					id: txtNWeight2
					text: runCmd.getCompositeValue(1, nWeight)
					type: SetInputField.Type.WeightType
					showLabel: false
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"
					visible: bCompositeExercise

					onVisibleChanged: cboSetType.currentIndex = visible ? 4 : 0
					onValueChanged:(str)=> nWeight = runCmd.setCompositeValue_QML(1, str, nWeight);
				}
			}

			Label {
				text: qsTr("Set type: ")
				font.bold: true
				Layout.leftMargin: 5
			}

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				Layout.bottomMargin: 10
				spacing: 1

				TPComboBox {
					id: cboSetType
					currentIndex: tDayModel.setType(0, exerciseIdx)

					onActivated: (index)=> {
						switch(index) {
							case 2: nSets = "1"; break; //DropSet
							case 3: nSets = "2"; break; //ClusterSet
							case 5: nSets = "3"; break; //MyoReps
							default: break;
						}
						if (bCompositeExercise) {
							if (index !== 4) {
								var exercisename = tDayModel.exerciseName1(exerciseIdx);
								exercisename = exercisename.replace("1: ", "");
								txtExerciseName.text = exercisename;
								bCompositeExercise = false;
							}
						}
						else {
							if (index === 4)
								bCompositeExercise = true;
						}
					}
				}

				SetInputField {
					id: txtNSets
					text: nSets
					type: SetInputField.Type.SetType
					availableWidth: layoutMain.width / 3
					alternativeLabels: ["","","",qsTr("sets #:")]
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str)=> nSets = str;
				}

				TPRoundButton {
					id: btnAddSet
					imageName: "add-new.png"
					focus: true
					Layout.minimumHeight: 40
					Layout.maximumHeight: 40
					Layout.minimumWidth: 40
					Layout.maximumWidth: 40
					Layout.leftMargin: 20

					onClicked: {
						createSetObject(cboSetType.currentIndex, parseInt(nSets), nReps, nWeight);
						requestFloatingButton(exerciseIdx, cboSetType.currentIndex, (setNbr + 1).toString());
					}
				}
			} // RowLayout
			ColumnLayout {
				id: exerciseSetsLayout
				objectName: "exerciseSetsLayout"
				spacing: 0
				Layout.fillWidth: true
				Layout.fillHeight: true
			}
		} // ColumnLayout layoutMain
	} //paneExercise

	Component.onCompleted: tDayModel.compositeExerciseChanged.connect(compositeExerciseActions);

	function compositeExerciseActions() {
		const bCompositeExercise2 = bCompositeExercise;
		bCompositeExercise = tDayModel.compositeExercise(exerciseIdx);
		//When a composite exercise is formed, nWeight and nReps get a second value. If the user decides to revert back to
		//a simple exercise, those variables will be cluttered with useless info that will stream down to the sets being created
		//We catch that condition now and clear the variables;
		if (bCompositeExercise2 && !bCompositeExercise) {
			nWeight = runCmd.getCompositeValue(0, nWeight);
			nReps = runCmd.getCompositeValue(0, nReps);
		}
	}

	function changeExercise(fromList: bool) {
		var interruptSignals = true;
		if (bListRequestForExercise1) {
			if (fromList)
				itemManager.changeSetsExerciseLabels(exerciseIdx, 1, exercisesListModel.selectedEntriesValue(0, 1) + " - " + exercisesListModel.selectedEntriesValue(0, 2));
			else
				itemManager.changeSetsExerciseLabels(exerciseIdx, 1, tDayModel.exerciseName1(exerciseIdx), false);
			bListRequestForExercise1 = false;
		}
		else if (bListRequestForExercise2) {
			if (fromList)
				itemManager.changeSetsExerciseLabels(exerciseIdx, 2, exercisesListModel.selectedEntriesValue(0, 1) + " - " + exercisesListModel.selectedEntriesValue(0, 2));
			else
				itemManager.changeSetsExerciseLabels(exerciseIdx, 2, tDayModel.exerciseName2(exerciseIdx), false);
			bListRequestForExercise2 = false;
		}
		else
		{
			interruptSignals = false;
			if (fromList)
				tDayModel.changeExerciseName(exerciseIdx, exercisesListModel);
			else
			{
				if (bListRequestForExercise1)
					itemManager.changeSetsExerciseLabels(exerciseIdx, 1, tDayModel.exerciseName1(exerciseIdx), false);
				else if (bListRequestForExercise2)
					itemManager.changeSetsExerciseLabels(exerciseIdx, 2, tDayModel.exerciseName2(exerciseIdx), false);
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
			itemManager.moveExercise(exerciseIdx, up ? --exerciseIdx : ++exerciseIdx);
		else {
			if (up) --exerciseIdx
			else ++exerciseIdx;
		}

		lblExerciseNumber.text = parseInt(exerciseIdx + 1) + ":";
		txtExerciseName.text = tDayModel.exerciseName(exerciseIdx);
		exerciseItem.Layout.row = exerciseIdx;
	}

	function createSetObject(type: int, n: int, nreps: string, nweight: string) {
		itemManager.createSetObjects(exerciseIdx, setNbr, setNbr + n, type, nreps, nweight);
		setNbr += n;
	}

	function paneExerciseShowHide(show: bool, force: bool) {
		paneExercise.shown = force ? show : !paneExercise.shown
		if (paneExercise.shown)
			itemManager.createSetObjects(exerciseIdx);
	}

	function liberateSignals(liberate: bool) {
		txtExerciseName.bCanEmitTextChanged = liberate;
	}
} //Item
