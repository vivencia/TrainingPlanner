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
	required property DBWorkoutModel workoutModel

	signal exerciseSelectedFromSimpleExercisesList();
	signal simpleExercisesListClosed();
	signal silenceTimeWarning();

	ScrollView {
		id: scrollTraining
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: layoutMain.height + exercisesLayout.implicitHeight

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
				text: WorkoutManager.headerText
				font: AppGlobals.largeFont
				horizontalAlignment: Text.AlignHCenter
				Layout.minimumWidth: workoutPage.width - 20
				Layout.maximumWidth: workoutPage.width - 20
			}

			TPLabel {
				text: WorkoutManager.muscularGroup
				horizontalAlignment: Text.AlignHCenter
				Layout.minimumWidth: workoutPage.width - 20
				Layout.maximumWidth: workoutPage.width - 20
			}

			RowLayout {
				Layout.fillWidth: true

				TPLabel {
					text: qsTr("Training Division:")
				}

				TPComboBox {
					id: cboSplitLetter
					model: AppGlobals.splitModel
					enabled: WorkoutManager.timerActive ? false : WorkoutManager.workoutIsEditable
					Layout.maximumWidth: 100

					Component.onCompleted: currentIndex = Qt.binding(function() { return cboSplitLetter.indexOfValue(WorkoutManager.splitLetter); });
					onActivated: (index) => WorkoutManager.splitLetter = valueAt(index);
				} //TPComboBox
			}

			RowLayout {
				visible: WorkoutManager.splitLetter !== "R"
				Layout.maximumWidth: workoutPage.width - 20

				TPLabel {
					text: qsTr("Location:")
					Layout.row: 1
					Layout.column: 0
				}
				TPTextInput {
					id: txtLocation
					placeholderText: WorkoutManager.lastWorkOutLocation
					text: WorkoutManager.lastWorkOutLocation
					enabled: WorkoutManager.workoutIsEditable
					Layout.row: 1
					Layout.column: 1
					Layout.fillWidth: true

					onTextChanged: WorkoutManager.lastWorkOutLocation = text;
				}
			}

			Frame {
				id: frmTrainingTime
				visible: WorkoutManager.splitLetter !== "R"
				enabled: WorkoutManager.timerActive ? false : !WorkoutManager.dayIsFinished
				height: appSettings.pageHeight*0.4
				Layout.preferredWidth: workoutPage.width - 20

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
							WorkoutManager.prepareWorkOutTimer();
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
								bOnlyFutureTime: WorkoutManager.mainDateIsToday ? WorkoutManager.editMode : false
								parentPage: workoutPage

								onTimeSet: (hour, minutes) => WorkoutManager.prepareWorkOutTimer(appUtils.getCurrentTimeString(), hour + ":" + minutes);
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
						enabled: optFreeTimeSession.checked && (WorkoutManager.editMode || WorkoutManager.mainDateIsToday)
						padding: 0
						spacing: 10
						Layout.fillWidth: true
						Layout.leftMargin: 5

						TPLabel {
							id: lblInTime
							text: qsTr("In time:")
							width: parent.width*0.35
						}

						TPTextInput {
							id: txtInTime
							text: WorkoutManager.timeIn
							horizontalAlignment: Text.AlignHCenter
							readOnly: true
							width: parent.width*0.25
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

								onTimeSet: (hour, minutes) => WorkoutManager.timeIn = hour + ":" + minutes;
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
						enabled: optFreeTimeSession.checked && (WorkoutManager.editMode || WorkoutManager.mainDateIsToday)
						padding: 0
						spacing: 10
						Layout.fillWidth: true
						Layout.leftMargin: 5

						TPLabel {
							id: lblOutTime
							text: qsTr("Out time:")
							width: parent.width*0.35
						}

						TPTextInput {
							id: txtOutTime
							text: WorkoutManager.timeOut
							horizontalAlignment: Text.AlignHCenter
							readOnly: true
							width: parent.width*0.25
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
								bOnlyFutureTime: WorkoutManager.mainDateIsToday ? WorkoutManager.editMode : false

								onTimeSet: (hour, minutes) => WorkoutManager.timeOut = hour + ":" + minutes;
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
				text: WorkoutManager.dayNotes
				editable: WorkoutManager.workoutIsEditable
				visible: WorkoutManager.splitLetter !== "R"
				Layout.fillWidth: true

				onEditFinished: (new_text) => WorkoutManager.dayNotes = new_text;
			}

			TPButton {
				text: qsTr("Use this workout exercises as the default exercises plan for the division ") + WorkoutManager.splitLetter + qsTr( " of this mesocycle")
				flat: false
				rounded: false
				visible: WorkoutManager.dayIsFinished && WorkoutManager.hasExercises
				enabled: WorkoutManager.editMode;
				Layout.fillWidth: true
				Layout.bottomMargin: 10

				onClicked: WorkoutManager.exportWorkoutToSplitPlan();
			}

			TPLabel {
				id: lblExercisesStart
				text: qsTr("--- EXERCISES ---")
				font: AppGlobals.extraLargeFont
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				visible: WorkoutManager.splitLetter !== "R"
				height: 40
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
					visible: WorkoutManager.hasExercises
					enabled: WorkoutManager.workoutIsEditable ? true : WorkoutManager.editMode
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

		GridLayout {
			id: exercisesLayout
			objectName: "exercisesLayout"
			width: parent.width
			columns: 1

			anchors {
				left: parent.left
				right:parent.right
				top: layoutMain.bottom
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
				else if (Math.abs(contentItem.contentY - (phantomItem.y - exercisesLayout.y)) < 50) {
					navButtons.showUpButton = true;
					navButtons.showDownButton = false;
				}
				else {
					navButtons.showUpButton = true;
					navButtons.showDownButton = true;
				}
			}
		}

		function scrollToPos(y_pos): void {
			contentItem.contentY = y_pos;
		}

		function setScrollBarPosition(pos): void {
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

		function init(pos): void {
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
		visible: WorkoutManager.splitLetter !== "R"

		readonly property int buttonHeight: width * 0.3

		Row {
			id: workoutLengthRow
			height: parent.height*0.4
			spacing: 5
			anchors {
				left: parent.left
				leftMargin: 5
				top: parent.top
				right: parent.right
				rightMargin: 5
			}

			TPLabel {
				text: !WorkoutManager.dayIsFinished ? qsTr("Workout:") : qsTr("Workout session length: ")
				width: parent.width * 0.3
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				flat: false
				width: parent.width * 0.2
				visible: WorkoutManager.mainDateIsToday ? !WorkoutManager.dayIsFinished && !WorkoutManager.editMode : false
				enabled: !WorkoutManager.timerActive

				onClicked: WorkoutManager.startWorkout();
			}

			TPDigitalClock {
				id: hoursClock
				max: 24
				value: WorkoutManager.timerHour

			}
			Rectangle { color : appSettings.fontColor; width: 2; height: 35 }

			TPDigitalClock {
				id: minsClock
				max: 60
				value: WorkoutManager.timerMinute
			}
			Rectangle { color : appSettings.fontColor; width: 2; height: 35 }

			TPDigitalClock {
				id: secsClock
				max: 60
				value: WorkoutManager.timerSecond
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")
				flat: false
				width: parent.width * 0.2
				visible: btnStartWorkout.visible
				enabled: WorkoutManager.timerActive

				onClicked: WorkoutManager.stopWorkout();
			}
		}

		TPButton {
			id: btnFinishedDayOptions
			imageSource: "menu.png"
			rounded: false
			backgroundColor: appSettings.paneBackgroundColor
			flat: false
			width: height / 2
			height: dayInfoToolBar.buttonHeight
			visible: WorkoutManager.dayIsFinished || !WorkoutManager.mainDateIsToday || WorkoutManager.editMode

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
			width: width * 1.2
			height: dayInfoToolBar.buttonHeight
			visible: WorkoutManager.dayIsFinished && WorkoutManager.hasExercises

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
			visible: WorkoutManager.splitLetter !== "R"
			enabled: WorkoutManager.workoutIsEditable
			width: height * 1.2
			height: dayInfoToolBar.buttonHeight

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

	function changeComboModel(mesoSplit: string): void {
		cboModel.get(0).enabled = mesoSplit.indexOf('A') !== -1;
		cboModel.get(1).enabled = mesoSplit.indexOf('B') !== -1;
		cboModel.get(2).enabled = mesoSplit.indexOf('C') !== -1;
		cboModel.get(3).enabled = mesoSplit.indexOf('D') !== -1;
		cboModel.get(4).enabled = mesoSplit.indexOf('E') !== -1;
		cboModel.get(5).enabled = mesoSplit.indexOf('F') !== -1;
		cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(WorkoutManager.splitLetter);
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
					WorkoutManager.changeSplit(cboSplitLetter.valueAt(cboSplitLetter.currentIndex), changeSplitLetterDialog.customBoolProperty1);
				});
				changeSplitLetterDialog.button2Clicked.connect( function() { WorkoutManager.changeSplit("X"); });
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
					msgClearExercises.button1Clicked.connect(function () { WorkoutManager.clearExercises(); } );
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
				adjustCalendarBox.button1Clicked.connect(function() { WorkoutManager.adjustCalendar(WorkoutManager.splitLetter, adjustCalendarBox.customBoolProperty1); });
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
					button1Text: qsTr("Proceed"), customItemSource:"TPTDayIntentGroup.qml", customBoolProperty1: WorkoutManager.canImportFromSplitPlan,
					customStringProperty2: WorkoutManager.splitLetter, customModel: WorkoutManager.previousWorkoutsList,
					customBoolProperty2: WorkoutManager.canImportFromPreviousWorkout, customBoolProperty3: !WorkoutManager.hasExercises });
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
				WorkoutManager.getExercisesFromSplitPlan();
			break;
			case 2: //use previous day
				WorkoutManager.loadExercisesFromDate(intentDlg.customStringProperty1);
			break;
			case 3: //import from file
				WorkoutManager.importWorkout();
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

		sourceComponent:SimpleExercisesListPanel {
			parentPage: workoutPage
			onExerciseSelected: exerciseSelectedFromSimpleExercisesList();
			onListClosed: {
				simpleExercisesListClosed();
				simpleExercisesListLoader.active = false;
			}
		}
		property bool enableMultipleSelection
		onLoaded: showSimpleExercisesList(enableMultipleSelection);
	}

	function showSimpleExercisesList(multipleSel: bool): void {
		if (simpleExercisesListLoader.status === Loader.Ready) {
			exercisesPane.bEnableMultipleSelection = multipleSel;
			exercisesPane.open();
		}
		else {
			simpleExercisesListLoader.enableMultipleSelection = multipleSel;
			simpleExercisesListLoader.active = true;
		}
	}

	function hideSimpleExercisesList(): void {
		exercisesPane.close();
	}

	property TimerDialog dlgSessionLength: null
	function limitedTimeSessionTimer(): void {
		if (dlgSessionLength === null) {
			let component = Qt.createComponent("qrc:/qml/Dialogs/TimerDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				dlgSessionLength = component.createObject(workoutPage, { parentPage: workoutPage, timePickerOnly: true,
					windowTitle: qsTr("Length of this training session") });
				dlgSessionLength.onUseTime.connect(function(strtime) { WorkoutManager.prepareWorkOutTimer(strtime); } );
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

	property TPBalloonTip msgRemoveExercise: null
	function showRemoveExerciseMessage(exerciseidx: int, exercisename: string): void {
		if (msgRemoveExercise === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveExercise = component.createObject(workoutPage, { parentPage: workoutPage, title: qsTr("Remove Exercise?"),
						keepAbove: true, imageSource: "remove" } );
					msgRemoveExercise.button1Clicked.connect(function () { WorkoutManager.removeExercise(exerciseidx); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		msgRemoveExercise.message = exercisename + qsTr("\nThis action cannot be undone.");
		msgRemoveExercise.show(-1);
	}

	property TPBalloonTip msgRemoveSet: null
	function showRemoveSetMessage(setnumber: int): void {
		if (msgRemoveSet === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveSet = component.createObject(workoutPage, { parentPage: workoutPage, keepAbove: true, imageSource: "remove",
						message: qsTr("This action cannot be undone.") } );
					msgRemoveSet.button1Clicked.connect(function () { WorkoutManager.currentExercise().removeSetObject(false); } );
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
					resetWorkoutMsg.button1Clicked.connect(function () { WorkoutManager.resetWorkout(); } );
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
				navButtons.visible = Qt.binding(function() { return WorkoutManager.splitLetter !== "R"; });
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
				WorkoutManager.editMode = !WorkoutManager.editMode;
				optionsMenu.setMenuText(0, WorkoutManager.editMode ? qsTr("Done") : qsTr("Edit workout"));
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

			onButton1Clicked: WorkoutManager.exportWorkout(exportDlgLoader.bShare);
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
