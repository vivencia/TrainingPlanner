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
	id: trainingDayPage
	objectName: "trainingDayPage"

	required property date mainDate
	required property QmlItemManager itemManager
	required property DBTrainingDayModel tDayModel

	//C++ controlled properties
	property string timeIn
	property string timeOut
	property string headerText
	property string lastWorkOutLocation
	property bool bHasPreviousTDays
	property bool bHasMesoPlan
	property bool mainDateIsToday
	property bool bNeedActivation //only set to true when, in the same app session, a second or more training days are created
	property var previousTDays: []

	//Private QML properties
	property bool editMode: false
	property var timerDialogRequester: null
	property TPComplexDialog intentDlg: null
	property TPFloatingButton btnFloat: null
	property PageScrollButtons navButtons: null
	property TimerDialog timerDialog: null

	signal mesoCalendarChanged()

	onPageActivated: {
		if (bNeedActivation)
			itemManager.setCurrenttDay(mainDate)
	}

	onEditModeChanged: {
		if (optionsMenu !== null)
			optionsMenu.setMenuText(0, editMode ? qsTr("Done") : qsTr("Edit workout"));
	}

	TimePicker {
		id: dlgTimeIn
		hrsDisplay: appUtils.getHourOrMinutesFromStrTime(txtInTime.text)
		minutesDisplay: appUtils.getMinutesOrSeconsFromStrTime(txtInTime.text)
		parentPage: trainingDayPage

		onTimeSet: (hour, minutes) => {
			timeIn = hour + ":" + minutes;
			tDayModel.setTimeIn(timeIn);
			if (timeOut != "--:--")
				timeManuallyEdited();
		}
	}

	TimePicker {
		id: dlgTimeOut
		hrsDisplay: appUtils.getHourOrMinutesFromStrTime(txtOutTime.text)
		minutesDisplay: appUtils.getMinutesOrSeconsFromStrTime(txtOutTime.text)
		parentPage: trainingDayPage

		Component.onCompleted: {
			if (appUtils.areDatesTheSame(mainDate, new Date()))
				bOnlyFutureTime = true;
		}

		onTimeSet: (hour, minutes) => {
			timeOut = hour + ":" + minutes;
			tDayModel.setTimeOut(timeOut);
			timeManuallyEdited();
		}
	}
	function timeManuallyEdited() {
		if (editMode) {
			const workoutLenght = appUtils.calculateTimeDifference(timeIn, timeOut);
			updateTimer(workoutLenght.getHours(), workoutLenght.getMinutes(), workoutLenght.getSeconds());
			itemManager.setDayIsFinished(true);
		}
		else {
			if (appUtils.areDatesTheSame(mainDate, new Date())) {
				optTimeConstrainedSession.checked = true;
				workoutTimer.stopWatch = false;
				workoutTimer.prepareTimer(appUtils.calculateTimeDifference_str(appUtils.getCurrentTimeString(), timeOut));
			}
		}
	}

	property TimerDialog dlgSessionLength: null
	function openSessionLengthTimer() {
		if (dlgSessionLength === null) {
			var component = Qt.createComponent("qrc:/qml/Dialogs/TimerDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				dlgSessionLength = component.createObject(trainingDayPage, { parentPage: trainingDayPage, timePickerOnly: true,
					windowTitle: qsTr("Length of this training session") });
				dlgSessionLength.onUseTime.connect(function(strtime) { workoutTimer.stopWatch = false; workoutTimer.prepareTimer(strtime + ":00"); } );
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		dlgSessionLength.open();
	}

	TimePicker {
		id: dlgTimeEndSession
		hrsDisplay: appUtils.getHourFromCurrentTime()
		minutesDisplay: appUtils.getMinutesFromCurrentTime()
		bOnlyFutureTime: true
		parentPage: trainingDayPage

		onTimeSet: (hour, minutes) => {
			workoutTimer.stopWatch = false;
			workoutTimer.prepareTimer(appUtils.calculateTimeDifference_str(
					appUtils.getCurrentTimeString(), hour + ":" + minutes));
		}
	}

	TPTimer {
		id: workoutTimer
		stopWatch: true
		interval: 1000

		Component.onCompleted: {
			addWarningAtMinute(15);
			addWarningAtMinute(5);
			addWarningAtMinute(1);
		}
	}

	property TPBalloonTip msgRemoveExercise: null
	function showRemoveExerciseMessage(exerciseidx: int) {
		if (!appSettings.alwaysAskConfirmation) {
			itemManager.removeExerciseObject(exerciseidx);
			return;
		}

		if (msgRemoveExercise === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveExercise = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Remove Exercise?"),
						button1Text: qsTr("Yes"), button2Text: qsTr("No"), imageSource: "remove" } );
					msgRemoveExercise.button1Clicked.connect(function () { itemManager.removeExerciseObject(exerciseidx); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		msgRemoveExercise.message = tDayModel.exerciseName(exerciseidx) + qsTr("\nThis action cannot be undone.");
		msgRemoveExercise.show(-1);
	}

	property TPBalloonTip msgRemoveSet: null
	function showRemoveSetMessage(setnumber: int, exerciseidx: int) {
		if (!appSettings.alwaysAskConfirmation) {
			itemManager.removeSetObject(setnumber, exerciseidx);
			return;
		}

		if (msgRemoveSet === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveSet = component.createObject(trainingDayPage, { parentPage: trainingDayPage, imageSource: "remove",
						message: qsTr("This action cannot be undone."), button1Text: qsTr("Yes"), button2Text: qsTr("No") } );
					msgRemoveSet.button1Clicked.connect(function () { itemManager.removeSetObject(setnumber, exerciseidx); } );
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

	property TPBalloonTip msgClearExercises: null
	function showClearExercisesMessage() {
		if (msgClearExercises === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgClearExercises = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Clear exercises list?"),
						message: qsTr("All exercises changes will be removed"), button1Text: qsTr("Yes"), button2Text: qsTr("No"), imageSource: "revert-day.png" } );
					msgClearExercises.button1Clicked.connect(function () { itemManager.clearExercises(itemManager); } );
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

	property TPBalloonTip tipTimeWarn: null
	function displayTimeWarning(timeleft: string, bmin: bool) {
		if (tipTimeWarn === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					tipTimeWarn = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Attention!"),
						message: qsTr("All exercises changes will be removed"), button1Text: "OK", imageSource: "sound-off" } );
					tipTimeWarn.button1Clicked.connect(function () { workoutTimer.stopAlarmSound(); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		var timeout;
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

	property TPBalloonTip timerDlgMessage: null
	function showTimerDialogMessage() {
		if (timerDlgMessage === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					timerDlgMessage = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Attention!"),
						message: qsTr("Only one timer window can be opened at a time!"), highlightMessage: true,
						button1Text: "OK", imageSource: "time.png" } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		timerDlgMessage.showTimed(3000, 0);
	}

	property TPBalloonTip resetWorkoutMsg: null
	function resetWorkoutMessage() {
		if (resetWorkoutMsg === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					resetWorkoutMsg = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Reset workout?"),
						message: qsTr("Exercises will not be afected"), button1Text: qsTr("Yes"), button2Text: qsTr("No"), imageSource: "reset.png" } );
					tipTimeWarn.button1Clicked.connect(function () { itemManager.resetWorkout(); } );
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

	ScrollView {
		id: scrollTraining
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		contentWidth: trainingDayPage.width //stops bouncing to the sides
		contentHeight: colMain.height + exercisesLayout.implicitHeight
		anchors.fill: parent

		ColumnLayout {
			id: colMain
			width: parent.width
			spacing: 10

			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}

			TPLabel {
				id: lblHeader
				text: headerText
				font: AppGlobals.titleFont
				topPadding: 15
				bottomPadding: 0
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10
			}

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5

				TPLabel {
					text: qsTr("Training Division:")
				}

				TPComboBox {
					id: cboSplitLetter
					model: AppGlobals.splitModel
					enabled: workoutTimer.active ? false : !tDayModel.dayIsFinished
					Layout.maximumWidth: 100

					onActivated: (index) => {
						if (cboModel.get(index).value !== tDayModel.splitLetter) {
							if (tDayModel.exerciseCount > 0)
								showSplitLetterChangedDialog();
							else
								changeSplitLetter();
						}
					}			
				} //TPComboBox
			}

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5

				TPLabel {
					text: qsTr("Location:")
					visible: txtLocation.visible
					Layout.row: 1
					Layout.column: 0
				}
				TPTextInput {
					id: txtLocation
					placeholderText: lastWorkOutLocation
					text: tDayModel.location()
					visible: tDayModel.splitLetter !== "R"
					enabled: tDayModel.dayIsEditable
					Layout.row: 1
					Layout.column: 1
					Layout.fillWidth: true

					onTextChanged: tDayModel.setLocation(text);
				}
			}

			Frame {
				id: frmTrainingTime
				visible: tDayModel.splitLetter !== "R"
				enabled: workoutTimer.active ? false : editMode
				height: 330
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5

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
							if (checked) {
								workoutTimer.stopWatch = true;
								workoutTimer.prepareTimer(timeIn);
							}
						}
					}
					TPRadioButton {
						id: optTimeConstrainedSession
						text: qsTr("Time constrained session")
						checked: false
						Layout.fillWidth: true
					}

					RowLayout {
						Layout.fillWidth: true
						Layout.leftMargin: 30
						Layout.bottomMargin: 10

						TPButton {
							id: btnTimeLength
							text: qsTr("By duration")
							visible: optTimeConstrainedSession.checked
							Layout.alignment: Qt.AlignCenter
							onClicked: openSessionLengthTimer();
						}
						TPButton {
							id: btnTimeHour
							text: qsTr("By time of day")
							visible: optTimeConstrainedSession.checked
							Layout.alignment: Qt.AlignCenter
							onClicked: dlgTimeEndSession.open();
						}
					} //RowLayout

					TPLabel {
						id: lblInTime
						text: qsTr("In time:")
						bottomPadding: 15
						Layout.fillWidth: true
						Layout.leftMargin: 5

						TPTextInput {
							id: txtInTime
							text: timeIn
							readOnly: true
							anchors {
								top: parent.top
								topMargin: -5
								left: parent.left
								leftMargin: 80
							}
						}

						TPButton {
							id: btnInTime
							imageSource: "time.png"
							imageSize: 30
							enabled: editMode || !appUtils.areDatesTheSame(mainDate, new Date())

							anchors {
								top: parent.top
								topMargin: -5
								left: txtInTime.right
							}

							onClicked: dlgTimeIn.open();
						}
					} //Label

					TPLabel {
						id: lblOutTime
						text: qsTr("Out time:")
						bottomPadding: 10
						Layout.fillWidth: true
						Layout.leftMargin: 5

						TPTextInput {
							id: txtOutTime
							text: timeOut
							readOnly: true
							Layout.leftMargin: 5

							anchors {
								top: parent.top
								topMargin: -5
								left: parent.left
								leftMargin: 80
							}
						}

						TPButton {
							id: btnOutTime
							imageSource: "time.png"
							imageSize: 30

							anchors {
								top: parent.top
								topMargin: -5
								left: txtOutTime.right
							}

							onClicked: dlgTimeOut.open();
						}
					} // RowLayout
				} //ColumnLayout
			} //Frame

			SetNotesField {
				info: qsTr("This training session considerations:")
				text: tDayModel.dayNotes()
				readOnly: tDayModel.dayIsEditable//!tDayModel.dayIsFinished
				visible: tDayModel.splitLetter !== "R"
				foreColor: appSettings.fontColor
				Layout.leftMargin: 5

				onEditFinished: (new_text) => tDayModel.setDayNotes(new_text);
			}

			TPButton {
				text: qsTr("Use this workout exercises as the default exercises plan for the division ") + tDayModel.splitLetter + qsTr( " of this mesocycle")
				flat: false
				rounded: false
				visible: tDayModel.dayIsFinished && tDayModel.exerciseCount > 0
				width: parent.width - 10
				fixedSize: true
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				Layout.bottomMargin: 10
				Layout.topMargin: -10

				onClicked: {
					itemManager.convertTDayToPlan();
					enabled = editMode;
				}
			}

			TPLabel {
				id: lblExercisesStart
				text: qsTr("--- EXERCISES ---")
				font: AppGlobals.titleFont
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				visible: tDayModel.splitLetter !== "R"
				height: 40
				Layout.bottomMargin: 10
				Layout.fillWidth: true
				Layout.minimumHeight: height
				Layout.maximumHeight: height

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
					imageSize: 20
					enabled: tDayModel.dayIsEditable
					visible: exercisesLayout.children.length > 0
					ToolTip.text: "Remove all exercises"

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
						leftMargin: 5
					}

					onClicked: showClearExercisesMessage();
				}
			}
		}// colMain

		GridLayout {
			id: exercisesLayout
			objectName: "tDayExercisesLayout"
			width: parent.width
			columns: 1

			anchors {
				left: parent.left
				leftMargin: 5
				rightMargin: 5
				right:parent.right
				top: colMain.bottom
			}
		}

		Item {
			id: phantomItem
			width: parent.width
			height: 10

			anchors {
				left: parent.left
				right:parent.right
				bottom: parent.bottom
			}
		}

		ScrollBar.vertical.onPositionChanged: {
			if (navButtons) {
				if (contentItem.contentY <= 50) {
					navButtons.showUpButton = false;
					navButtons.showDownButton = true;
				}
				else if (Math.abs(contentItem.contentY - (phantomItem.y - lblExercisesStart.y)) < 50) {
					navButtons.showUpButton = true;
					navButtons.showDownButton = false;
				}
				else {
					navButtons.showUpButton = true;
					navButtons.showDownButton = true;
				}
			}
		}

		function scrollToPos(y_pos) {
			contentItem.contentY = y_pos;
		}

		function setScrollBarPosition(pos) {
			if (pos === 0)
				ScrollBar.vertical.setPosition(0);
			else
				ScrollBar.vertical.setPosition(pos - ScrollBar.vertical.size/2);
		}
	} // ScrollView scrollTraining

	Timer {
		id: scrollTimer
		interval: 200
		running: false
		repeat: false

		property int ypos:0

		onTriggered: scrollTraining.scrollToPos(ypos);

		function init(pos) {
			if (pos >= 0)
				ypos = exercisesLayout.y + pos;
			else
				ypos = 0;
			start();
		}
	}

	function changeComboModel() {
		const mesoSplit = mesocyclesModel.split(tDayModel.mesoIdx);
		cboModel.get(0).enabled = mesoSplit.indexOf('A') !== -1;
		cboModel.get(1).enabled = mesoSplit.indexOf('B') !== -1;
		cboModel.get(2).enabled = mesoSplit.indexOf('C') !== -1;
		cboModel.get(3).enabled = mesoSplit.indexOf('D') !== -1;
		cboModel.get(4).enabled = mesoSplit.indexOf('E') !== -1;
		cboModel.get(5).enabled = mesoSplit.indexOf('F') !== -1;
		cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(tDayModel.splitLetter);
	}

	footer: ToolBar {
		id: dayInfoToolBar
		width: parent.width
		height: 100
		visible: tDayModel.splitLetter !== "R"

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

		RowLayout {
			id: workoutLengthRow
			height: 50
			spacing: 5
			anchors {
				left: parent.left
				leftMargin: 5
				top: parent.top
				right: parent.right
				rightMargin: 5
			}

			TPLabel {
				text: !tDayModel.dayIsFinished ? qsTr("Workout:") : qsTr("Workout session length: ")
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				flat: false
				visible: mainDateIsToday ? !tDayModel.dayIsFinished && !editMode : false
				enabled: !workoutTimer.active

				onClicked: {
					if (timeIn.indexOf("-") === -1)
						workoutTimer.prepareTimer(appUtils.getCurrentTimeString());
					timeIn = appUtils.getCurrentTimeString();
					workoutTimer.startTimer(timeIn);
					tDayModel.setTimeIn(timeIn);
					tDayModel.dayIsEditable = true;
					workoutTimer.timeWarning.connect(displayTimeWarning);
					if (tDayModel.location().length === 0)
						 tDayModel.setLocation(txtLocation.placeholderText);
				}
			}

			Rectangle {
				id: workoutLengthClock
				antialiasing: true
				width: spinnerLayout.width
				height: 35
				color: appSettings.primaryColor

				RowLayout {
					id: spinnerLayout
					spacing: 2

					TPDigitalClock {
						id: hoursClock
						max: 24
						value: workoutTimer.hours
					}
					Rectangle { color : appSettings.fontColor; width: 2; height: 35 }

					TPDigitalClock {
						id: minsClock
						max: 60
						value: workoutTimer.minutes
					}
					Rectangle { color : appSettings.fontColor; width: 2; height: 35 }

					TPDigitalClock {
						id: secsClock
						max: 60
						value: workoutTimer.seconds
					}
				}
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")
				flat: false
				visible: btnStartWorkout.visible
				enabled: workoutTimer.active

				onClicked: {
					workoutTimer.stopTimer();
					const sessionLength = workoutTimer.elapsedTime();
					updateTimer(sessionLength.getHours(), sessionLength.getMinutes(), sessionLength.getSeconds());
					timeOut = appUtils.getCurrentTimeString();
					tDayModel.setTimeOut(timeOut);
					tDayModel.dayIsEditable = false;
					itemManager.setDayIsFinished(true);
				}
			}
		}

		TPButton {
			id: btnFinishedDayOptions
			imageSource: "menu.png"
			backgroundColor: appSettings.primaryDarkColor
			rounded: false
			flat: false
			fixedSize: true
			width: 55
			height: 55
			visible: tDayModel.dayIsFinished || !mainDateIsToday

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
			fixedSize: true
			width: 70
			height: 55
			visible: tDayModel.dayIsFinished

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
			height: 55
			visible: tDayModel.splitLetter !== "R"
			enabled: tDayModel.dayIsEditable

			anchors {
				right: parent.right
				rightMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: itemManager.getExercisesPage(true, trainingDayPage);
		} // bntAddExercise
	} //footer: ToolBar

	SimpleExercisesListPanel {
		id: exercisesPane
		parentPage: trainingDayPage
	}

	TPComplexDialog {
		id: adjustCalendarBox
		title: qsTr("Re-adjust meso calendar?")
		customStringProperty1: lblHeader.text
		customStringProperty2: qsTr("Only alter this day")
		customStringProperty3: "calendar.png"
		button1Text: qsTr("Adjust")
		button2Text: qsTr("Cancel")
		customItemSource: "TPDialogWithMessageAndCheckBox.qml"
		parentPage: trainingDayPage

		property string newSplitLetter

		onButton1Clicked: itemManager.adjustCalendar(newSplitLetter, adjustCalendarBox.customBoolProperty1);
		onButton2Clicked: cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(tDayModel.splitLetter);
	}

	function changeSplitLetter() {
		if (cboSplitLetter.currentValue !== tDayModel.splitLetter) {
			adjustCalendarBox.newSplitLetter = cboSplitLetter.currentValue;
			adjustCalendarBox.show(-1);
		}
	}

	function showIntentionDialog() {
		if (!intentDlg) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				intentDlg = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title:qsTr("What do you want to do today?"),
					button1Text: qsTr("Proceed"), customItemSource:"TPTDayIntentGroup.qml", bClosable: false, customBoolProperty1: bHasMesoPlan,
					customStringProperty2: tDayModel.splitLetter, customModel: previousTDays,
					customBoolProperty2: bHasPreviousTDays, customBoolProperty3: tDayModel.exerciseCount === 0 });
				intentDlg.button1Clicked.connect(intentChosen);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		intentDlg.show(-1);
	}

	function intentChosen() {
		switch (intentDlg.customIntProperty1) {
			case 1: //use meso plan
				itemManager.loadExercisesFromMesoPlan();
			break;
			case 2: //use previous day
				itemManager.loadExercisesFromDate(intentDlg.customStringProperty1);
			break;
			case 3: //import from file
				itemManager.importTrainingDay();
			break;
			case 4: //empty session
				bHasPreviousTDays = false;
				bHasMesoPlan = false;
			break;
		}
	}

	function gotExercise() {
		function readyToProceed(object, id) {
			createNavButtons();
			scrollTimer.init(phantomItem.y);
			return;
		}
		itemManager.createExerciseObject();
	}

	function placeSetIntoView(ypos: int) {
		scrollTimer.init(ypos);
	}

	function createFloatingAddSetButton(exerciseIdx: int, settype: int, nset: string) {
		var component = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingButton.qml", Qt.Asynchronous);
		function finishCreation() {
			btnFloat = component.createObject(trainingDayPage, { text:qsTr("Add set"), parentPage: trainingDayPage,
					image:"add-new.png", exerciseIdx:exerciseIdx, comboIndex:settype });
			btnFloat.updateDisplayText(nset);
			btnFloat.buttonClicked.connect(createNewSet);
			btnFloat.visible = true;
		}
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function requestFloatingButton(exerciseIdx: int, settype: int, nset: string) {
		if (btnFloat === null)
			createFloatingAddSetButton(exerciseIdx, settype, nset);
		else {
			btnFloat.exerciseIdx = exerciseIdx;
			btnFloat.comboIndex = settype;
			btnFloat.updateDisplayText(nset);
			btnFloat.visible = true;
		}
	}

	function createNewSet(settype, exerciseidx) {
		itemManager.createSetObject(settype, tDayModel.setsNumber(exerciseidx), exerciseidx, true, "", "");
		btnFloat.updateDisplayText(parseInt(tDayModel.setsNumber(exerciseidx)) + 1);
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		exercisesPane.itemThatRequestedSimpleList = visible ? object : null;
		exercisesPane.bEnableMultipleSelection = multipleSel;
		exercisesPane.visible = visible;
	}

	function requestTimerDialog(requester, message, mins, secs) {
		if (timerDialog === null) {
			var component = Qt.createComponent("qrc:/qml/Dialogs/TimerDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				timerDialog = component.createObject(trainingDayPage, { parentPage: trainingDayPage, bJustMinsAndSecs:true, simpleTimer:false });
				timerDialog.onUseTime.connect(timerDialogUseButtonClicked);
				timerDialog.onClosed.connect(timerDialogClosed);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		if (!timerDialog.visible) {
			timerDialogRequester = requester;
			timerDialog.windowTitle = message;
			timerDialog.initialTime = "00:" + mins + ":" + secs;
			if (timerDlgMessage)
				timerDlgMessage.close();
			timerDialog.open();
		}
		else
			showTimerDialogMessage();
	}

	function timerDialogUseButtonClicked(strTime) {
		timerDialogRequester.timeChanged(strTime);
	}

	function timerDialogClosed() {
		timerDialogRequester = null;
		if (timerDlgMessage)
			timerDlgMessage.close();
	}

	function createNavButtons() {
		if (navButtons === null) {
			var component = Qt.createComponent("qrc:/qml/ExercisesAndSets/PageScrollButtons.qml", Qt.Asynchronous);

			function finishCreation() {
				navButtons = component.createObject(trainingDayPage, { ownerPage: trainingDayPage });
				navButtons.scrollTo.connect(scrollTraining.setScrollBarPosition);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	function updateTimer(hour: int, min: int, sec: int)
	{
		hoursClock.value = hour;
		minsClock.value = min;
		secsClock.value = sec;
	}

	function resetTimer() {
		hoursClock.value = Qt.binding(function() { return workoutTimer.hours; });
		minsClock.value = Qt.binding(function() { return workoutTimer.minutes; });
		secsClock.value = Qt.binding(function() { return workoutTimer.seconds; });
		workoutTimer.resetTimer(false);
	}

	property TPFloatingMenuBar optionsMenu: null
	function showFinishedWorkoutOptions() {
		if (optionsMenu === null) {
			var optionsMenuMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			optionsMenu = optionsMenuMenuComponent.createObject(trainingDayPage, { parentPage: trainingDayPage });
			optionsMenu.addEntry(qsTr("Edit workout"), "edit.png", 0, true);
			optionsMenu.addEntry(qsTr("Reset Workout"), "reset.png", 1, appUtils.areDatesTheSame(mainDate, new Date()));
			optionsMenu.menuEntrySelected.connect(selectedOptionsMenuOption);
		}
		optionsMenu.show(btnFinishedDayOptions, 0);
	}

	function selectedOptionsMenuOption(menuid) {
		switch (menuid) {
			case 0:
				if (!editMode)
					btnFinishedDayOptions.visible = true;
				else {
					itemManager.setDayIsFinished(true);
					btnFinishedDayOptions.visible = Qt.binding(function() { return tDayModel.dayIsFinished; });
				}
				editMode = !editMode;
				tDayModel.dayIsEditable = editMode;
			break;
			case 1:
				resetWorkoutMessage();
			break;
		}
	}

	property TPComplexDialog changeSplitLetterDialog: null
	function showSplitLetterChangedDialog() {
		if (!changeSplitLetterDialog) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				changeSplitLetterDialog = component.createObject(trainingDayPage, { parentPage: trainingDayPage, button1Text: qsTr("Yes"),
					button2Text: qsTr("No"), customStringProperty1: qsTr("Really change split?"), customStringProperty2: qsTr("Clear exercises list?"),
					customStringProperty3: "remove", customItemSource:"TPDialogWithMessageAndCheckBox.qml" });
				changeSplitLetterDialog.button1Clicked.connect( function() {
					if (changeSplitLetterDialog.customBoolProperty1)
						itemManager.clearExercises();
					changeSplitLetter();
				} );
				changeSplitLetterDialog.button2Clicked.connect( function() {
					cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(tDayModel.splitLetter === "" ?
													splitLetter : tDayModel.splitLetter)
				} );
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		changeSplitLetterDialog.show(-1);
	}

	property TPFloatingMenuBar exportMenu: null
	readonly property bool bExportEnabled: tDayModel.dayIsFinished && tDayModel.exerciseCount > 0

	onBExportEnabledChanged: {
		if (Qt.platform.os === "android") {
			if (exportMenu) {
				exportMenu.enableMenuEntry(0, bExportEnabled);
				exportMenu.enableMenuEntry(1, bExportEnabled);
			}
		}
	}

	function showExportMenu() {
		if (exportMenu === null) {
			var exportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			exportMenu = exportMenuComponent.createObject(trainingDayPage, { parentPage: trainingDayPage });
			exportMenu.addEntry(qsTr("Export"), "save-day.png", 0, true);
			exportMenu.addEntry(qsTr("Share"), "export.png", 1, true);
			exportMenu.menuEntrySelected.connect(function(id) { exportTypeTip.init(id === 1); });
		}
		exportMenu.show(btnImExport, 0);
	}

	TPBalloonTip {
		id: exportTypeTip
		title: bShare ? qsTr("Share workout?") : qsTr("Export workout to file?")
		imageSource: "export.png"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		parentPage: trainingDayPage

		onButton1Clicked: itemManager.exportTrainingDay(bShare, tDayModel);

		property bool bShare

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}
} // Page
