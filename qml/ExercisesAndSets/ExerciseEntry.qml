import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

FocusScope {
	id: exerciseItem
	implicitHeight: paneExercise.height
	Layout.fillWidth: true

	required property ExerciseEntryManager exerciseManager

	Frame {
		id: paneExercise
		visible: height > 0
		height: shown ? implicitHeight : (exerciseManager.hasSets ? txtExerciseName.height + 10 : layoutMain.implicitHeight + 20)
		implicitHeight: layoutMain.implicitHeight + exerciseSetsLayout.implicitHeight + 20
		implicitWidth: width
		width: parent.width
		clip: true
		padding: 0
		spacing: 0
		Layout.fillWidth: true

		property bool shown: false

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		background: Rectangle {
			color: exerciseManager.exerciseIdx % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
			border.color: "transparent"
			opacity: 0.8
			radius: 10
		}

		TPButton {
			id: btnMoveExerciseUp
			imageSource: "up"
			hasDropShadow: false
			enabled: exerciseManager.exerciseIdx > 0
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
			spacing: 0

			anchors {
				top: parent.top
				left: parent.left
				leftMargin: 5
				right: parent.right
				rightMargin: 5
			}

			RowLayout {
				spacing: 0
				Layout.fillWidth: true

				TPButton {
					id: btnFoldIcon
					imageSource: paneExercise.shown ? "black/fold-up" : "black/fold-down"
					hasDropShadow: false
					imageSize: 18
					onClicked: paneExerciseShowHide(!paneExercise.shown);
				}

				Label {
					id: lblExerciseNumber
					text: exerciseManager.exerciseNumber + ":"
					font.bold: true
					font.pixelSize: appSettings.fontSize
					width: 15
				}

				ExerciseNameField {
					id: txtExerciseName
					text: exerciseManager.exerciseName
					bEditable: exerciseManager.isEditable
					width: layoutMain.width*0.85
					height: appSettings.pageHeight*0.1
					Layout.preferredWidth: width
					Layout.preferredHeight: height

					Keys.onReturnPressed: txtNReps.forceActiveFocus();
					onExerciseChanged: (new_text) => exerciseManager.exerciseName = new_text;
					onRemoveButtonClicked: exerciseManager.removeExercise();
					onEditButtonClicked: exerciseManager.simpleExercisesList(!readOnly, true);
					onItemClicked: paneExerciseShowHide(!paneExercise.shown);
				}
			} //Row txtExerciseName

			RowLayout {
				id: trackRestTimeRow
				enabled: exerciseManager.isEditable && exerciseManager.canEditRestTimeTracking
				spacing: 0
				Layout.fillWidth: true

				TPCheckBox {
					id: chkTrackRestTime
					text: qsTr("Track rest times?")
					textColor: "black"
					checked: exerciseManager.trackRestTime
					width: layoutMain.width*0.45
					Layout.preferredWidth: width

					onClicked: exerciseManager.trackRestTime = checked;
				}

				TPCheckBox {
					id: chkAutoRestTime
					text: qsTr("Auto tracking")
					textColor: "black"
					enabled: exerciseManager.trackRestTime
					checked: exerciseManager.autoRestTime
					width: layoutMain.width*0.45
					Layout.preferredWidth: width

					onPressAndHold: ToolTip.show(qsTr("Tap on Start Rest/Stop Rest to have the rest time automatically recorded"), 5000);
					onClicked: exerciseManager.autoRestTime = checked;
				}
			}

			RowLayout {
				enabled: exerciseManager.isEditable
				spacing: 0
				Layout.fillWidth: true

				SetInputField {
					id: txtNReps
					text: exerciseManager.repsForExercise1
					type: SetInputField.Type.RepType
					availableWidth: layoutMain.width*0.45
					backColor: "transparent"
					borderColor: "transparent"
					Layout.preferredWidth: width

					onValueChanged: (str) => exerciseManager.repsForExercise1 = str;
					onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
				}

				SetInputField {
					id: txtNWeight
					text: exerciseManager.weightForExercise1
					type: SetInputField.Type.WeightType
					availableWidth: layoutMain.width*0.45
					backColor: "transparent"
					borderColor: "transparent"
					Layout.preferredWidth: width

					onValueChanged: (str) => exerciseManager.weightForExercise1 = str;
					onEnterOrReturnKeyPressed: !exerciseManager.compositeExercise ? txtNSets.forceActiveFocus() : txtNReps2.forceActiveFocus();
				}
			}

			RowLayout {
				enabled: exerciseManager.isEditable
				visible: exerciseManager.compositeExercise
				spacing: 0
				Layout.fillWidth: true

				SetInputField {
					id: txtNReps2
					text: exerciseManager.repsForExercise2
					type: SetInputField.Type.RepType
					availableWidth: layoutMain.width*0.45
					backColor: "transparent"
					borderColor: "transparent"
					Layout.preferredWidth: width

					onValueChanged: (str) => exerciseManager.repsForExercise2 = str;
					onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
				}

				SetInputField {
					id: txtNWeight2
					text: exerciseManager.weightForExercise2
					type: SetInputField.Type.WeightType
					availableWidth: layoutMain.width*0.45
					backColor: "transparent"
					borderColor: "transparent"
					Layout.preferredWidth: width

					onValueChanged: (str) => exerciseManager.weightForExercise2 = str;
					onEnterOrReturnKeyPressed: txtNSets.forceActiveFocus();
				}
			}

			SetInputField {
				id: txtRestTime
				type: SetInputField.Type.TimeType
				text: exerciseManager.restTime
				availableWidth: layoutMain.width*0.45
				backColor: "transparent"
				borderColor: "transparent"
				enabled: exerciseManager.trackRestTime && !exerciseManager.autoRestTime
				Layout.preferredWidth: width

				onValueChanged: (str) => exerciseManager.restTime = str;
			}

			Label {
				text: qsTr("Set type: ")
				font.bold: true
			}

			RowLayout {
				enabled: exerciseManager.isEditable
				spacing: 0
				Layout.fillWidth: true

				TPComboBox {
					id: cboSetType
					currentIndex: exerciseManager.newSetType
					model: AppGlobals.setTypesModel
					width: layoutMain.width*0.45
					Layout.preferredWidth: width

					onActivated: (index) => exerciseManager.newSetType = index;
				}

				SetInputField {
					id: txtNSets
					text: exerciseManager.setsNumber
					type: SetInputField.Type.SetType
					availableWidth: layoutMain.width*0.3
					alternativeLabels: ["","","",qsTr("sets #:")]
					backColor: "transparent"
					borderColor: "transparent"
					Layout.preferredWidth: width

					onValueChanged: (str)=> exerciseManager.setsNumber = str;
				}

				TPButton {
					id: btnAddSet
					imageSource: "add-new"
					imageSize: 30
					Layout.leftMargin: 40

					onClicked: exerciseManager.appendNewSet();
				}
			} // RowLayout
		} // ColumnLayout layoutMain

		GridLayout {
			id: exerciseSetsLayout
			objectName: "exerciseSetsLayout"
			width: parent.width
			columns: 1

			anchors {
				top: layoutMain.bottom
				topMargin: 10
				left: parent.left
				leftMargin: 5
				right:parent.right
				rightMargin: 5
			}
		}
	} //paneExercise

	function paneExerciseShowHide(show: bool): void {
		paneExercise.shown = show;
		if (show)
			exerciseManager.createAvailableSets();
	}
} //Item
