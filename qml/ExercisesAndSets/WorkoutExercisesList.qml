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
		(exerciseManager.hasSets ? txtExerciseName.height + 10 : layoutMain.implicitHeight + 20)
	swipe.enabled: enabled && !showSets
	clip: true
	padding: 5
	spacing: 5
	Layout.fillWidth: true

	required property ExerciseEntryManager exerciseManager
	property bool showSets: false

	readonly property Item setsLayout: exerciseSetsLayout

	Behavior on height {
		NumberAnimation {
			easing.type: Easing.InOutBack
		}
	}

	background: Rectangle {
		color: exerciseManager.exerciseNumber % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
		border.color: "transparent"
		opacity: 0.8
		radius: 10
	}

	TPButton {
		id: btnMoveExerciseUp
		imageSource: "up.png"
		hasDropShadow: false
		enabled: exerciseManager.exerciseNumber > 0
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
				imageSource: showSets ? "fold-up.png" : "fold-down.png"
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
				editable: false
				showEditButton: false
				width: layoutMain.width*0.85
				height: appSettings.pageHeight*0.1
				Layout.preferredWidth: width
				Layout.preferredHeight: height

				onRemoveButtonClicked: exerciseManager.removeExercise();
				onItemClicked: paneExerciseShowHide(!showSets);
				onItemPressed: if (enabled && !showSets) swipe.open(SwipeDelegate.Right);
			}
		} //Row txtExerciseName

		RowLayout {
			id: trackRestTimeRow
			enabled: exerciseManager.isEditable && exerciseManager.canEditRestTimeTracking
			spacing: 0
			Layout.fillWidth: true
			Layout.topMargin: 15

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
	} // ColumnLayout layoutMain

	ColumnLayout {
		id: subExerciseAndSetsLayout
		width: parent.width

		anchors {
				top: layoutMain.bottom
				topMargin: 10
				left: parent.left
				leftMargin: 5
				right:parent.right
				rightMargin: 5
		}

		Repeater {
			id: subExerciseRepeater
			model: exerciseManager.subExercisesCount
			Layout.fillWidth: true

			delegate: ColumnLayout {
				required property int index
				spacing: 5

				ExerciseNameField {
					text: exerciseManager.exerciseName(index)
					editable: exerciseManager.isEditable
					width: layoutMain.width*0.85
					height: appSettings.pageHeight*0.1
					Layout.preferredWidth: width
					Layout.preferredHeight: height

					onExerciseChanged: (new_text) => exerciseManager.setExerciseName(index, new_text);
					onRemoveButtonClicked: exerciseManager.removeExercise();
					onEditButtonClicked: exerciseManager.simpleExercisesList(!readOnly, true);
					onItemClicked: paneExerciseShowHide(!showSets);
					onItemPressed: if (enabled && !showSets) swipe.open(SwipeDelegate.Right);
				}

				GridLayout {
					columns: 1
					columnSpacing: 0
					rowSpacing: 5
					width: parent.width
				}

				TPButton {
					id: btnCompleteExercise
					text: qsTr("Exercise completed")
					flat: false
					visible: exerciseManager.isEditable
					enabled: exerciseManager.allSetsCompleted
					height: visible ? 30 : 0

					onClicked: exerciseManager.allSetsCompleted = true;
				}
			}
		} //Repeater
	}

	swipe.right: Rectangle {
		width: parent.width
		height: parent.height - 30
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

	function getLayoutForSubExercise(exercise_idx: int): GridLayout {
		return subExerciseRepeater.itemAt(exercise_idx).children[1];
	}
} //Item
