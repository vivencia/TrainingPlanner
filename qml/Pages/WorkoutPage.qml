import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "../"
import "../Dialogs"
import "../ExercisesAndSets"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: workoutPage
	objectName: "workoutPage"

	required property WorkoutManager workoutManager
	property DBWorkoutModel workoutModel

	signal exerciseSelectedFromSimpleExercisesList();
	signal silenceTimeWarning();

	Connections {
		target: workoutModel
		function onExerciseCountChanged() : void {
			scrollTraining.setScrollBarPosition(1);
			lstWorkoutExercises.positionViewAtIndex(workoutModel.workingExercise, ListView.Contain);
		}
	}

	onPageActivated: cboSplitLetter.currentIndex = Qt.binding(function() { return cboSplitLetter.indexOfValue(workoutModel.splitLetter); })

	ScrollView {
		id: scrollTraining
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: layoutMain.height + lstWorkoutExercises.height
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

		ScrollBar.vertical: ScrollBar {
			id: vBar
			policy: ScrollBar.AsNeeded
			active: ScrollBar.AsNeeded
			visible: lstWorkoutExercises.contentHeight > lstWorkoutExercises.height

			onPositionChanged: {
				//if (navButtons) {
					//const absoluteScrollY =  vBar.position * (scrollTraining.contentHeight - scrollTraining.height);
					//if (absoluteScrollY <= 50) {
					if ((vBar.position - (1 - vBar.size)) < -0.279) {
						navButtons.showUpButton = false;
						navButtons.showDownButton = true;
					}
					else if (vBar.position - (1 - vBar.size) > -0.029) {
						navButtons.showUpButton = true;
						navButtons.showDownButton = false;
					}
					else {
						navButtons.showUpButton = true;
						navButtons.showDownButton = true;
					}
				//}
			}
		}

		function setScrollBarPosition(pos: int): void {
			if (pos === 0)
				vBar.setPosition(0);
			else
				vBar.setPosition(pos - vBar.size/2);
		}

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 10
			bottomMargin: 10
		}

		ColumnLayout {
			id: layoutMain
			width: parent.width
			spacing: 10

			anchors {
				top: parent.top
				left: parent.left
				leftMargin: 5
				right: parent.right
				rightMargin: 5
			}

			TPLabel {
				id: lblHeader
				text: workoutManager.headerText
				font: AppGlobals.largeFont
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
			}

			TPLabel {
				text: workoutManager.headerText_2
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
			}

			RowLayout {
				Layout.fillWidth: true

				TPLabel {
					text: qsTr("Training Division:")
				}

				TPComboBox {
					id: cboSplitLetter
					model: AppGlobals.splitModel
					enabled: workoutManager.timerActive ? false : !workoutManager.dayIsFinished
					onActivated: (index) => workoutManager.changeSplitLetter(valueAt(index));
				} //TPComboBox
			}

			Row {
				visible: workoutModel.splitLetter !== "R"
				Layout.fillWidth: true
				spacing: 0
				padding: 0

				TPLabel {
					text: qsTr("Location:")
					width: parent.width * 0.25
				}
				TPTextInput {
					id: txtLocation
					placeholderText: workoutManager.lastWorkOutLocation()
					text: workoutManager.lastWorkOutLocation()
					enabled: workoutManager.timerActive ? false : !workoutManager.dayIsFinished
					width: parent.width * 0.75

					onTextChanged: workoutManager.lastWorkOutLocation = text;
				}
			}

			Frame {
				id: frmTrainingTime
				visible: workoutModel.splitLetter !== "R"
				enabled: workoutManager.timerActive ? false : !workoutManager.dayIsFinished
				height: appSettings.pageHeight*0.4
				Layout.fillWidth: true

				background: Rectangle {
					border.color: appSettings.fontColor
					color: "transparent"
					radius: 6
				}

				ColumnLayout {
					id: timeLayout
					anchors.fill: parent

					TPRadioButton {
						id: optFreeTimeSession
						text: qsTr("Open time training session")
						checked: true
						Layout.fillWidth: true

						onClicked: {
							workoutManager.prepareWorkOutTimer();
							optTimeConstrainedSession.checked = false;
						}
					}

					TPRadioButton {
						id: optTimeConstrainedSession
						text: qsTr("Time constrained session")
						checked: false
						Layout.fillWidth: true

						onClicked: optFreeTimeSession.checked = false;
					}

					RowLayout {
						visible: optTimeConstrainedSession.checked
						Layout.fillWidth: true
						Layout.leftMargin: 30
						Layout.bottomMargin: 10

						TPButton {
							id: btnTimeLength
							text: qsTr("By duration")
							autoSize: true
							Layout.alignment: Qt.AlignCenter

							onClicked: limitedTimeSessionTimer();
						}

						TPButton {
							id: btnTimeHour
							text: qsTr("By time of day")
							autoSize: true
							Layout.alignment: Qt.AlignCenter

							onClicked: restrictedTimeLoader.openDlg();
						}

						Loader {
							id: restrictedTimeLoader
							active: false
							asynchronous: true

							sourceComponent: TimePicker {
								id: dlgTimeEndSession
								hrsDisplay: appUtils.getHourFromCurrentTime()
								minutesDisplay: appUtils.getMinutesFromCurrentTime()
								bOnlyFutureTime: workoutManager.mainDateIsToday ? workoutManager.editMode : false
								parentPage: workoutPage

								onTimeSet: (hour, minutes) => workoutManager.prepareWorkOutTimer(appUtils.getCurrentTimeString(), hour + ":" + minutes);
								onClosed: restrictedTimeLoader.active = false;
							}

							onLoaded: openDlg();

							function openDlg(): void {
								if (status === Loader.Ready)
									item.open();
								else
									active = true;
							}
						}
					} //RowLayout

					Row {
						enabled: optFreeTimeSession.checked && (workoutManager.editMode || workoutManager.mainDateIsToday)
						padding: 0
						spacing: 10
						Layout.fillWidth: true
						Layout.leftMargin: 5

						TPLabel {
							id: lblInTime
							text: qsTr("In time:")
							width: parent.width * 0.35
						}

						TPTextInput {
							id: txtInTime
							text: workoutManager.timeIn
							horizontalAlignment: Text.AlignHCenter
							readOnly: true
							width: parent.width * 0.25
						}

						TPButton {
							id: btnInTime
							imageSource: "time.png"
							width: appSettings.itemDefaultHeight
							height: width

							onClicked: timeInLoader.openDlg();
						}

						Loader {
							id: timeInLoader
							active: false
							asynchronous: true

							sourceComponent: TimePicker {
								id: dlgTimeIn
								hrsDisplay: appUtils.getHourFromStrTime(txtInTime.text)
								minutesDisplay: appUtils.getMinutesFromStrTime(txtInTime.text)
								parentPage: workoutPage

								onTimeSet: (hour, minutes) => workoutManager.timeIn = hour + ":" + minutes;
								onClosed: timeInLoader.active = false;
							}

							onLoaded: openDlg();

							function openDlg(): void {
								if (status === Loader.Ready)
									item.open();
								else
									active = true;
							}
						}
					}

					Row {
						enabled: optFreeTimeSession.checked && (workoutManager.editMode || workoutManager.mainDateIsToday)
						padding: 0
						spacing: 10
						Layout.fillWidth: true
						Layout.leftMargin: 5

						TPLabel {
							id: lblOutTime
							text: qsTr("Out time:")
							width: parent.width * 0.35
						}

						TPTextInput {
							id: txtOutTime
							text: workoutManager.timeOut
							horizontalAlignment: Text.AlignHCenter
							readOnly: true
							width: parent.width * 0.25
						}

						TPButton {
							id: btnOutTime
							imageSource: "time.png"
							width: appSettings.itemDefaultHeight
							height: width

							onClicked: timeOutLoader.openDlg();
						}

						Loader {
							id: timeOutLoader
							active: false
							asynchronous: true

							sourceComponent: TimePicker {
								id: dlgTimeOut
								hrsDisplay: appUtils.getHourFromStrTime(txtOutTime.text)
								minutesDisplay: appUtils.getMinutesFromStrTime(txtOutTime.text)
								parentPage: workoutPage
								bOnlyFutureTime: workoutManager.mainDateIsToday ? workoutManager.editMode : false

								onTimeSet: (hour, minutes) => workoutManager.timeOut = hour + ":" + minutes;
								onClosed: timeOutLoader.active = false;
							}

							onLoaded: openDlg();

							function openDlg(): void {
								if (status === Loader.Ready)
									item.open();
								else
									active = true;
							}
						}
					}
				} //ColumnLayout
			} //Frame

			SetNotesField {
				info: qsTr("This training session considerations:")
				//text: workoutManager.dayNotes
				editable: workoutManager.workoutIsEditable
				visible: workoutModel.splitLetter !== "R"
				Layout.fillWidth: true

				onEditFinished: (new_text) => workoutManager.dayNotes = new_text;
			}

			TPButton {
				text: qsTr("Use this workout exercises as the default exercises plan for the division ") + workoutModel.splitLetter + qsTr( " of this mesocycle")
				flat: false
				rounded: false
				visible: workoutManager.dayIsFinished && workoutManager.hasExercises
				enabled: workoutManager.editMode;
				Layout.fillWidth: true
				Layout.minimumHeight: 3 * appSettings.itemDefaultHeight
				Layout.bottomMargin: 10

				onClicked: workoutManager.exportWorkoutToSplitPlan();
			}

			TPLabel {
				id: lblExercisesStart
				text: qsTr("--- EXERCISES ---")
				font: AppGlobals.extraLargeFont
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				visible: workoutModel.splitLetter !== "R"
				height: appSettings.largeFontSize * 1.2
				Layout.bottomMargin: 10
				Layout.fillWidth: true
				Layout.preferredHeight: height

				background: Rectangle {
					gradient: Gradient {
						orientation: Gradient.Horizontal
						GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
						GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
						GradientStop { position: 0.50; color: appSettings.primaryColor; }
						GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
					}
					opacity: 0.8
				}

				TPButton {
					id: btnClearExercises
					imageSource: "revert-day.png"
					width: appSettings.itemDefaultHeight
					height: width
					visible: workoutManager.hasExercises
					enabled: workoutManager.workoutIsEditable ? true : workoutManager.editMode
					ToolTip.text: "Remove all exercises"

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
						leftMargin: 5
					}

					onClicked: showClearExercisesMessage();
				}
			}
		}// layoutMain

		WorkoutExercisesList {
			id: lstWorkoutExercises
			currentIndex: workoutModel.workingExercise
			height: contentHeight

			anchors {
				top: layoutMain.bottom
				left: parent.left
				right: parent.right
			}
		}
	} // ScrollView scrollTraining

	Timer {
		id: scrollTimer
		interval: 200
		running: false
		repeat: false

		property int ypos: 0

		onTriggered: scrollTraining.scrollToPos(ypos);

		function init(pos: int): void {
			if (pos >= 0)
				ypos = pos;
			else
				ypos = 0;
			start();
		}
	}

	footer: TPToolBar {
		id: dayInfoToolBar
		height: appSettings.pageHeight*0.18
		visible: workoutModel.splitLetter !== "R"

		readonly property int buttonHeight: width * 0.3

		Row {
			id: workoutLengthRow
			height: parent.height*0.4
			topPadding: 5
			bottomPadding: 5
			leftPadding: 0
			rightPadding: 0
			spacing: 10

			anchors {
				left: parent.left
				leftMargin: 5
				top: parent.top
				right: parent.right
				rightMargin: 5
			}

			TPLabel {
				text: !workoutManager.dayIsFinished ? qsTr("Workout:") : qsTr("Workout session length: ")
				width: parent.width * 0.2
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				flat: false
				width: parent.width * 0.2
				height: appSettings.itemDefaultHeight
				visible: workoutManager.mainDateIsToday ? !workoutManager.dayIsFinished && !workoutManager.editMode : false
				enabled: !workoutManager.timerActive

				onClicked: workoutManager.startWorkout();
			}

			TPDigitalClock {
				id: hoursClock
				max: 24
				value: workoutManager.timerHour

			}
			Rectangle { color : appSettings.fontColor; width: 2; height: 35 }

			TPDigitalClock {
				id: minsClock
				max: 60
				value: workoutManager.timerMinute
			}
			Rectangle { color : appSettings.fontColor; width: 2; height: 35 }

			TPDigitalClock {
				id: secsClock
				max: 60
				value: workoutManager.timerSecond
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")
				flat: false
				width: parent.width * 0.2
				height: appSettings.itemDefaultHeight
				visible: btnStartWorkout.visible
				enabled: workoutManager.timerActive

				onClicked: workoutManager.stopWorkout();
			}
		}

		TPButton {
			id: btnFinishedDayOptions
			imageSource: "menu.png"
			rounded: false
			backgroundColor: appSettings.paneBackgroundColor
			flat: false
			width: height * 0.6
			height: dayInfoToolBar.buttonHeight
			visible: workoutManager.dayIsFinished || !workoutManager.mainDateIsToday || workoutManager.editMode

			anchors {
				left: parent.left
				leftMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: showFinishedWorkoutOptions();
		}

		TPButton {
			id: btnExport
			text: qsTr("Export")
			imageSource: "import-export.png"
			textUnderIcon: true
			rounded: false
			flat: false
			width: parent.width * 0.3
			height: parent.height * 0.5
			visible: workoutManager.dayIsFinished && workoutManager.hasExercises

			anchors {
				left: btnFinishedDayOptions.right
				leftMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: {
				if (Qt.platform.os === "android")
					showExportMenu();
				else
					exportTypeTip.init(false);
			}
		}

		TPButton {
			id: btnAddExercise
			text: qsTr("Add exercise")
			imageSource: "exercises-add.png"
			rounded: false
			textUnderIcon: true
			flat: false
			visible: workoutModel.splitLetter !== "R"
			//enabled: workoutManager.workoutIsEditable
			width: parent.width * 0.35
			height: parent.height * 0.5

			anchors {
				right: parent.right
				rightMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: workoutManager.addExercise();
		} // bntAddExercise
	} //footer: ToolBar

	function setViewingExercise(exercise_number: int) : void {
		lstWorkoutExercises.positionViewAtIndex(exercise_number, ListView.Contain);
	}

	function changeComboModel(mesoSplit: string): void {
		cboModel.get(0).enabled = mesoSplit.indexOf('A') !== -1;
		cboModel.get(1).enabled = mesoSplit.indexOf('B') !== -1;
		cboModel.get(2).enabled = mesoSplit.indexOf('C') !== -1;
		cboModel.get(3).enabled = mesoSplit.indexOf('D') !== -1;
		cboModel.get(4).enabled = mesoSplit.indexOf('E') !== -1;
		cboModel.get(5).enabled = mesoSplit.indexOf('F') !== -1;
		cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(workoutModel.splitLetter);
	}

	property TPComplexDialog changeSplitLetterDialog: null
	function showSplitLetterChangedDialog(): void {
		if (!changeSplitLetterDialog) {
			let component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				changeSplitLetterDialog = component.createObject(workoutPage, { parentPage: workoutPage,
					customStringProperty1: qsTr("Really change split?"), customStringProperty2: qsTr("Clear exercises list?"),
					customStringProperty3: "remove", customItemSource:"TPDialogWithMessageAndCheckBox.qml" });
				changeSplitLetterDialog.button1Clicked.connect( function() {
					workoutManager.changeSplit(cboSplitLetter.valueAt(cboSplitLetter.currentIndex), changeSplitLetterDialog.customBoolProperty1);
				});
				changeSplitLetterDialog.button2Clicked.connect( function() { workoutManager.changeSplit("X"); });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		changeSplitLetterDialog.show(-1);
	}

	property TPBalloonTip msgClearExercises: null
	function showClearExercisesMessage(): void {
		if (msgClearExercises === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgClearExercises = component.createObject(workoutPage, { parentPage: workoutPage, title: qsTr("Clear exercises list?"),
						keepAbove: true, message: qsTr("All exercises changes will be removed"), imageSource: "revert-day.png" } );
					msgClearExercises.button1Clicked.connect(function () { workoutManager.clearExercises(); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		msgClearExercises.show(-1);
	}

	property TPComplexDialog adjustCalendarBox: null
	function showAdjustCalendarDialog(): void {
		if (!adjustCalendarBox) {
			let component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				adjustCalendarBox = component.createObject(workoutPage, { parentPage: workoutPage, title: qsTr("Re-adjust meso calendar?"),
					button1Text: qsTr("Adjust"), button2Text: qsTr("Cancel"), customItemSource:"TPDialogWithMessageAndCheckBox.qml",
					customStringProperty1: lblHeader.text, customStringProperty2: qsTr("Only alter this day"), customStringProperty3: "calendar.png" });
				adjustCalendarBox.button1Clicked.connect(function() { workoutManager.adjustCalendar(workoutModel.splitLetter, adjustCalendarBox.customBoolProperty1); });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		adjustCalendarBox.show(-1);
	}

	property TPComplexDialog intentDlg: null
	function showIntentionDialog(): void {
		if (!intentDlg) {
			let component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				intentDlg = component.createObject(workoutPage, { parentPage: workoutPage, title: qsTr("What do you want to do today?"),
					button1Text: qsTr("Proceed"), customItemSource:"TPTDayIntentGroup.qml", customBoolProperty1: workoutManager.canImportFromSplitPlan,
					customStringProperty2: workoutModel.splitLetter, customModel: workoutManager.previousWorkoutsList,
					customBoolProperty2: workoutManager.canImportFromPreviousWorkout, customBoolProperty3: !workoutManager.hasExercises });
				intentDlg.button1Clicked.connect(intentChosen);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		intentDlg.show(-1);
	}

	function intentChosen(): void {
		switch (intentDlg.customIntProperty1) {
			case 1: //use meso plan
				workoutManager.getExercisesFromSplitPlan();
			break;
			case 2: //use previous day
				workoutManager.loadExercisesFromDate(intentDlg.customStringProperty1);
			break;
			case 3: //import from file
				workoutManager.importWorkout();
			break;
			case 4: //empty session
			break;
		}
	}

	function placeSetIntoView(ypos: int): void {
		if (ypos === -1)
			ypos = phantomItem.y - lblExercisesStart.y;
		else if (ypos === -2)
			ypos = phantomItem.y;
		else
			ypos += exercisesLayout.y;
		scrollTimer.init(ypos);
	}

	property alias exercisesPane: simpleExercisesListLoader.item
	Loader {
		id: simpleExercisesListLoader
		active: false
		asynchronous: true

		sourceComponent: SimpleExercisesListPanel {
			parentPage: workoutPage
			onExerciseSelected: exerciseSelectedFromSimpleExercisesList();
			onListClosed: simpleExercisesListLoader.active = false;
		}
		property bool enableMultipleSelection
		onLoaded: exercisesPane.show(lstWorkoutExercises.exerciseNameFieldYPosition() > appSettings.pageHeight/2 ? 0 : -2)
	}

	function showSimpleExercisesList(): void {
		simpleExercisesListLoader.active = true;
	}

	function hideSimpleExercisesList(): void {
		exercisesPane.visible = false;
	}

	property TimerDialog dlgSessionLength: null
	function limitedTimeSessionTimer(): void {
		if (dlgSessionLength === null) {
			let component = Qt.createComponent("qrc:/qml/Dialogs/TimerDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				dlgSessionLength = component.createObject(workoutPage, { parentPage: workoutPage, timePickerOnly: true,
					windowTitle: qsTr("Length of this training session") });
				dlgSessionLength.onUseTime.connect(function(strtime) { workoutManager.prepareWorkOutTimer(strtime); } );
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		dlgSessionLength.open();
	}

	property TPBalloonTip tipTimeWarn: null
	function displayTimeWarning(timeleft: string, bmin: bool): void {
		if (tipTimeWarn === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					tipTimeWarn = component.createObject(workoutPage, { parentPage: workoutPage, title: qsTr("Attention!"),
						message: qsTr("All exercises changes will be removed"), button1Text: "OK", imageSource: "sound-off" } );
					tipTimeWarn.button1Clicked.connect(function () { silenceTimeWarning(); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		let timeout;
		if (!bmin)
		{
			timeout = 60000;
			workoutTimer.setAlarmSoundLoops(4);
		}
		else
			timeout = 18000;
		tipTimeWarn.message = "<b>" + timeleft + (bmin ? qsTr(" minutes") : qsTr(" seconds")) + qsTr("</b> until end of training session!");
		tipTimeWarn.showTimed(timeout, 0);
	}

	property TPBalloonTip generalMessage: null
	function showMessageDialog(title: string, message: string, error: bool, msecs: int): void {
		if (generalMessage === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					generalMessage = component.createObject(workoutPage, { parentPage: workoutPage, highlightMessage: true, button1Text: "OK"});
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		generalMessage.title = title;
		generalMessage.message = message;
		generalMessage.imageSource = error ? "error" : "warning";
		if (msecs > 0)
			generalMessage.showTimed(msecs, 0);
		else
			generalMessage.show(0);

	}

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise?")
		message: exerciseName + qsTr("\nThis action cannot be undone.")
		imageSource: "remove"
		keepAbove: true
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		onButton1Clicked: workoutManager.removeExercise();
		parentPage: workoutPage

		property string exerciseName

		function init(exercise: string): void {
			exerciseName = exercise;
			show(-1);
		}
	} //TPBalloonTip

	function showDeleteDialog(exercise: string): void {
		msgDlgRemove.init(exercise);
	}

	property TPBalloonTip msgRemoveSet: null
	function showRemoveSetMessage(setnumber: int): void {
		if (msgRemoveSet === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveSet = component.createObject(workoutPage, { parentPage: workoutPage, keepAbove: true, imageSource: "remove",
						message: qsTr("This action cannot be undone.") } );
					msgRemoveSet.button1Clicked.connect(function () { workoutManager.currentExercise().removeSetObject(false); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		msgRemoveSet.title = qsTr("Remove set #") + parseInt(setnumber + 1) + "?"
		msgRemoveSet.show(-1);
	}

	property TPBalloonTip resetWorkoutMsg: null
	function resetWorkoutMessage(): void {
		if (resetWorkoutMsg === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					resetWorkoutMsg = component.createObject(workoutPage, { parentPage: workoutPage, title: qsTr("Reset workout?"),
						message: qsTr("Exercises will not be afected"), imageSource: "reset.png" } );
					resetWorkoutMsg.button1Clicked.connect(function () { workoutManager.resetWorkout(); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		resetWorkoutMsg.show(-1);
	}

	property PageScrollButtons navButtons: null
	function createNavButtons(): void {
		if (navButtons === null) {
			let component = Qt.createComponent("qrc:/qml/ExercisesAndSets/PageScrollButtons.qml", Qt.Asynchronous);

			function finishCreation() {
				navButtons = component.createObject(workoutPage, { ownerPage: workoutPage });
				navButtons.scrollTo.connect(scrollTraining.setScrollBarPosition);
				navButtons.visible = Qt.binding(function() { return workoutModel.splitLetter !== "R"; });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	property TPFloatingMenuBar optionsMenu: null
	function showFinishedWorkoutOptions(): void {
		if (optionsMenu === null) {
			let optionsMenuMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			optionsMenu = optionsMenuMenuComponent.createObject(workoutPage, { parentPage: workoutPage });
			optionsMenu.addEntry(qsTr("Edit workout"), "edit.png", 0, true);
			optionsMenu.addEntry(qsTr("Reset Workout"), "reset.png", 1, true);
			optionsMenu.menuEntrySelected.connect(selectedOptionsMenuOption);
		}
		optionsMenu.show2(btnFinishedDayOptions, 0);
	}

	function selectedOptionsMenuOption(menuid): void {
		switch (menuid) {
			case 0:
				workoutManager.editMode = !workoutManager.editMode;
				optionsMenu.setMenuText(0, workoutManager.editMode ? qsTr("Done") : qsTr("Edit workout"));
			break;
			case 1:
				resetWorkoutMessage();
			break;
		}
	}

	property TPFloatingMenuBar exportMenu: null
	function showExportMenu(): void {
		if (exportMenu === null) {
			let exportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			exportMenu = exportMenuComponent.createObject(workoutPage, { parentPage: workoutPage });
			exportMenu.addEntry(qsTr("Export"), "save-day.png", 0, true);
			if (Qt.platform.os === "android")
				exportMenu.addEntry(qsTr("Share"), "export.png", 1, true);
			exportMenu.menuEntrySelected.connect(function(id) { exportDlgLoader.openDlg(id === 1); });
		}
		exportMenu.show2(btnExport, 0);
	}

	Loader {
		id: exportDlgLoader
		active: false
		asynchronous: true
		sourceComponent: TPBalloonTip {
			id: exportTypeTip
			title: bShare ? qsTr("Share workout?") : qsTr("Export workout to file?")
			imageSource: "export.png"
			parentPage: workoutPage
			closeButtonVisible: true

			onButton1Clicked: workoutManager.exportWorkout(exportDlgLoader.bShare);
			onClosed: exportDlgLoader.active = false;
		}
		property bool bShare
		onLoaded: openDlg(bShare);

		function openDlg(share: bool): void {
			if (status === Loader.Ready) {
				bShare = share;
				item.show(-1);
			}
			else
				active = true;
		}
	}
} // Page
