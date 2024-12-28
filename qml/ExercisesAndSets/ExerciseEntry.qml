import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

SwipeDelegate {
	id: paneExercise
	visible: height > 0
	implicitHeight: showSets ? layoutMain.implicitHeight + exerciseSetsLayout.height + 20 :
		(exerciseManager.hasSets ? txtExerciseName.height : layoutMain.implicitHeight + 20)
	swipe.enabled: enabled && !showSets
	clip: true
	padding: 5
	spacing: 5
	Layout.fillWidth: true

	required property ExerciseEntryManager exerciseManager
	property bool showSets: false


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
		imageSource: "up.png"
		hasDropShadow: false
		enabled: exerciseManager.exerciseIdx > 0
		visible: exerciseManager.isEditable
		height: 30
		width: 30
		opacity: 1 + swipe.position

		anchors {
			left: parent.left
			leftMargin: 0
			top: parent.top
			topMargin: 5
		}

		onClicked: exerciseManager.moveExerciseUp();
	}

	TPButton {
		id: btnMoveExerciseDown
		imageSource: "down.png"
		hasDropShadow: false
		enabled: !exerciseManager.lastExercise
		visible: exerciseManager.isEditable
		height: 30
		width: 30
		opacity: 1 + swipe.position

		anchors {
			left: parent.left
			leftMargin: 20
			top: parent.top
			topMargin: 5
		}

		onClicked: exerciseManager.moveExerciseDown();
	}

	ColumnLayout {
		id: layoutMain
		spacing: 0
		opacity: 1 + swipe.position

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		RowLayout {
			spacing: 0
			Layout.fillWidth: true
			Layout.leftMargin: 5

			TPButton {
				id: btnFoldIcon
				imageSource: showSets ? "fold-up" : "fold-down"
				hasDropShadow: false
				imageSize: 18
				Layout.preferredWidth: 18
				Layout.preferredHeight: 18
				onClicked: paneExerciseShowHide(!showSets);
			}

			TPLabel {
				id: lblExerciseNumber
				text: exerciseManager.exerciseNumber + ":"
				font.bold: true
				font.pixelSize: appSettings.fontSize
				Layout.leftMargin: 5
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
				onItemClicked: paneExerciseShowHide(!showSets);
				onItemPressed: if (enabled && !showSets) swipe.open(SwipeDelegate.Right);
			}
		} //Row txtExerciseName

		RowLayout {
			id: trackRestTimeRow
			enabled: exerciseManager.isEditable && exerciseManager.canEditRestTimeTracking
			spacing: 0
			Layout.fillWidth: true
			Layout.topMargin: 10

			TPCheckBox {
				id: chkTrackRestTime
				text: qsTr("Track rest times?")
				checked: exerciseManager.trackRestTime
				width: layoutMain.width*0.45
				Layout.preferredWidth: width

				onClicked: exerciseManager.trackRestTime = checked;
			}

			TPCheckBox {
				id: chkAutoRestTime
				text: qsTr("Auto tracking")
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
			Layout.topMargin: 10

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
				Layout.leftMargin: 5

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
				Layout.leftMargin: 5

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

		Row {
			enabled: exerciseManager.isEditable
			spacing: 10
			Layout.fillWidth: true

			TPLabel {
				text: qsTr("Set type: ")
				width: parent.width*0.3
			}

			TPComboBox {
				id: cboSetType
				currentIndex: exerciseManager.newSetType
				model: AppGlobals.setTypesModel
				width: parent.width*0.65

				onActivated: (index) => exerciseManager.newSetType = index;
			}
		}

		Row {
			enabled: exerciseManager.isEditable
			spacing: 10
			Layout.fillWidth: true

			TPLabel {
				text: qsTr("Add new set(s):")
				width: parent.width*0.4
			}

			SetInputField {
				id: txtNSets
				text: exerciseManager.setsNumber
				type: SetInputField.Type.SetType
				availableWidth: layoutMain.width*0.35
				showLabel: false
				backColor: "transparent"
				borderColor: "transparent"

				onValueChanged: (str)=> exerciseManager.setsNumber = str;
			}

			TPButton {
				id: btnAddSet
				imageSource: "add-new"
				imageSize: 30

				onClicked: exerciseManager.appendNewSet();
			}
		}
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

	swipe.right: Rectangle {
		width: parent.width
		height: parent.height
		clip: false
		color: SwipeDelegate.pressed ? "#555" : "#666"
		radius: 10

		TPImage {
			source: "remove"
			width: 30
			height: 30
			opacity: 2 * -swipe.position

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
			}
		}
	} //swipe.right

	swipe.onCompleted: exerciseManager.removeExercise();

	function paneExerciseShowHide(show: bool): void {
		if (show && exerciseManager.hasSets)
			exerciseManager.createAvailableSets();
		showSets = show;
	}
} //Item
