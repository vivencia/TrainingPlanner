import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import com.vivenciasoftware.qmlcomponents

FocusScope {
	id: exerciseItem
	Layout.fillWidth: true
	implicitHeight: paneExercise.height

	property DBTrainingDayModel tDayModel
	property int exerciseIdx

	property int setNbr: 0
	property string nSets
	property string nReps
	property string nWeight
	property bool bListRequestForExercise1: false
	property bool bListRequestForExercise2: false

	signal requestSimpleExercisesList(Item requester, var bVisible, var bMultipleSelection, int id)
	signal requestFloatingButton(var exerciseidx, var settype, var nset)

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise")
		message: tDayModel.exerciseName(exerciseIdx) + qsTr("? This action cannot be undone.")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		onButton1Clicked: itemManager.removeExerciseObject(exerciseIdx);
	} //TPBalloonTip

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
			radius: 5
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
			objectName: "exerciseSetsLayout"
			anchors.fill: parent
			spacing: 0
			enabled: !tDayModel.dayIsFinished

			Row {
				spacing: 0
				padding: 0
				Layout.fillWidth: true
				Layout.topMargin: 10

				TPRoundButton {
					id: btnFoldIcon
					height: 25
					width: 25
					imageName: paneExercise.shown ? "fold-up.png" : "fold-down.png"
					onClicked: paneExerciseShowHide(false);
					z: 1
				}

				Label {
					id: lblExerciseNumber
					text: parseInt(exerciseIdx + 1) + ":"
					font.pointSize: AppSettings.fontSizeText
					width: 15
					padding: 2
				}

				ExerciseNameField {
					id: txtExerciseName
					text: tDayModel.exerciseName(exerciseIdx)
					width: windowWidth - 65
					Layout.minimumWidth: width
					Layout.maximumWidth: width
					Layout.leftMargin: 45

					Keys.onReturnPressed: txtNReps.forceActiveFocus();

					onExerciseChanged: (new_text) => tDayModel.setExerciseName1(new_text, exerciseIdx);
					onRemoveButtonClicked: msgDlgRemove.show(exerciseItem.y)
					onEditButtonClicked: requestSimpleExercisesList(exerciseItem, !readOnly, cboSetType.currentIndex === 4, 1);
					onItemClicked: paneExerciseShowHide(false);
				}
			} //Row txtExerciseName

			Row {
				Layout.topMargin: 10

				SetInputField {
					id: txtNReps
					text: !txtNReps2.visible ? nReps : runCmd.getCompositeValue(0, nReps)
					type: SetInputField.Type.RepType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged:(str)=> nReps = runCmd.setCompositeValue(0, str, nWeight);
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
					visible: cboSetType.currentIndex === 4

					onValueChanged:(str)=> nReps = runCmd.setCompositeValue(1, str);
					onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
				}
			}

			Row {

				SetInputField {
					id: txtNWeight
					text: !txtNWeight2.visible ? nWeight : runCmd.getCompositeValue(0, nWeight)
					type: SetInputField.Type.WeightType
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged:(str)=> nWeight = runCmd.setCompositeValue(0, str, nWeight);
				}

				SetInputField {
					id: txtNWeight2
					text: runCmd.getCompositeValue(0, nWeight)
					type: SetInputField.Type.RepType
					showLabel: false
					availableWidth: layoutMain.width / 2
					backColor: "transparent"
					borderColor: "transparent"
					visible: cboSetType.currentIndex === 4

					onValueChanged:(str)=> nWeight = runCmd.setCompositeValue(0, str, nWeight);
				}
			}

			Label {
				text: qsTr("Set type: ")
				font.bold: true
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
					Layout.minimumHeight: 30
					Layout.maximumHeight: 30
					Layout.minimumWidth: 30
					Layout.maximumWidth: 30
					Layout.leftMargin: 20

					onClicked: {
						createSetObject(cboSetType.currentIndex, parseInt(nSets), nReps, nWeight);
						requestFloatingButton(exerciseIdx, cboSetType.currentIndex, (setNbr + 1).toString());
					}
				}
			} // RowLayout
		} // ColumnLayout layoutMain
	} //paneExercise

	function changeExercise(multipleSelectionOpt)
	{
		if (multipleSelectionOpt !== 0) return;
		if (bListRequestForExercise1) {
			itemManager.changeSetsExerciseLabels(exerciseIdx, 1, exercisesListModel.selectedEntriesValue(0, 1) + " - " + exercisesListModel.selectedEntriesValue(0, 2));
			bListRequestForExercise1 = false;
		}
		else if (bListRequestForExercise2) {
			itemManager.changeSetsExerciseLabels(exerciseIdx, 2, exercisesListModel.selectedEntriesValue(1, 1) + " - " + exercisesListModel.selectedEntriesValue(1, 2));
			bListRequestForExercise2 = false;
		}
		else
			tDayModel.setExerciseName(exercisesListModel.selectedEntriesValue(0, 1) + " - " + exercisesListModel.selectedEntriesValue(0, 2), exerciseIdx);
		txtExerciseName.text = tDayModel.exerciseName(exerciseIdx);
		requestSimpleExercisesList(null, false, false, 1);
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
		exerciseItem.Layout.row = exerciseIdx;
	}

	function createSetObject(type: int, n: int, nreps: string, nweight: string) {
		itemManager.createSetObjects(exerciseIdx, setNbr, setNbr + n, type, nreps, nweight);
		setNbr += n;
	}

	function paneExerciseShowHide(force: bool) {
		paneExercise.shown = !force ? !paneExercise.shown : true
		if (paneExercise.shown)
			itemManager.createSetObjects(exerciseIdx);
	}

	function setSetComboIndex(new_index: int) {
		cboSetType.currentIndex = new_index;
	}
} //Item
