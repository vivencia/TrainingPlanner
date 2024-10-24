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

	required property TDayManager tDayManager

	//C++ controlled properties
	property bool bHasPreviousTDays
	property bool bHasMesoPlan
	property var previousTDays: []

	signal exerciseSelectedFromSimpleExercisesList();
	signal simpleExercisesListClosed();
	signal silenceTimeWarning();

	TimePicker {
		id: dlgTimeIn
		hrsDisplay: appUtils.getHourOrMinutesFromStrTime(txtInTime.text)
		minutesDisplay: appUtils.getMinutesOrSeconsFromStrTime(txtInTime.text)
		parentPage: trainingDayPage

		onTimeSet: (hour, minutes) => tDayManager.timeIn = hour + ":" + minutes;
	}

	TimePicker {
		id: dlgTimeOut
		hrsDisplay: appUtils.getHourOrMinutesFromStrTime(txtOutTime.text)
		minutesDisplay: appUtils.getMinutesOrSeconsFromStrTime(txtOutTime.text)
		parentPage: trainingDayPage
		bOnlyFutureTime: tDayManager.mainDateIsToday

		onTimeSet: (hour, minutes) => tDayManager.timeOut = hour + ":" + minutes;
	}

	TimePicker {
		id: dlgTimeEndSession
		hrsDisplay: appUtils.getHourFromCurrentTime()
		minutesDisplay: appUtils.getMinutesFromCurrentTime()
		bOnlyFutureTime: tDayManager.mainDateIsToday
		parentPage: trainingDayPage

		onTimeSet: (hour, minutes) => tDayManager.prepareWorkOutTimer(appUtils.getCurrentTimeString(), hour + ":" + minutes);
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
				text: tDayManager.headerText
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
					enabled: tDayManager.timerActive ? false : !tDayManager.dayIsFinished
					currentIndex: indexOfValue(tDayManager.splitLetter)
					Layout.maximumWidth: 100

					onActivated: (index) => tDayManager.splitLetter = valueAt(index);
				} //TPComboBox
			}

			RowLayout {
				visible: tDayManager.splitLetter !== "R"
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5

				TPLabel {
					text: qsTr("Location:")
					Layout.row: 1
					Layout.column: 0
				}
				TPTextInput {
					id: txtLocation
					placeholderText: tDayManager.lastWorkOutLocation
					text: tDayManager.lastWorkOutLocation
					enabled: tDayManager.dayIsEditable
					Layout.row: 1
					Layout.column: 1
					Layout.fillWidth: true

					onTextChanged: tDayManager.lastWorkOutLocation = text;
				}
			}

			Frame {
				id: frmTrainingTime
				visible: tDayManager.splitLetter !== "R"
				enabled: tDayManager.timerActive ? false : tDayManager.editMode
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

						onClicked: tDayManager.prepareWorkOutTimer();
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

							onClicked: limitedTimeSessionTimer();
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
							text: tDayManager.timeIn
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
							enabled: tDayManager.editMode || tDayManager.mainDateIsToday

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
							text: tDayManager.timeOut
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
							enabled: tDayManager.editMode || tDayManager.mainDateIsToday

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
				text: tDayManager.dayNotes
				readOnly: !tDayManager.dayIsEditable
				visible: tDayManager.splitLetter !== "R"
				foreColor: appSettings.fontColor
				Layout.leftMargin: 5

				onEditFinished: (new_text) => tDayManager.dayNotes = new_text;
			}

			TPButton {
				text: qsTr("Use this workout exercises as the default exercises plan for the division ") + tDayManager.splitLetter + qsTr( " of this mesocycle")
				flat: false
				rounded: false
				visible: tDayManager.dayIsFinished && tDayManager.hasExercises
				enabled: tDayManager.editMode;
				width: parent.width - 10
				fixedSize: true
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				Layout.bottomMargin: 10
				Layout.topMargin: -10

				onClicked: tDayManager.convertTDayToPlan();
			}

			TPLabel {
				id: lblExercisesStart
				text: qsTr("--- EXERCISES ---")
				font: AppGlobals.titleFont
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				visible: tDayManager.splitLetter !== "R"
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
					visible: tDayManager.hasExercises
					enabled: tDayManager.dayIsEditable
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

	footer: ToolBar {
		id: dayInfoToolBar
		width: parent.width
		height: 100
		visible: tDayManager.splitLetter !== "R"

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
				text: !tDayManager.dayIsFinished ? qsTr("Workout:") : qsTr("Workout session length: ")
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				flat: false
				visible: tDayManager.mainDateIsToday ? !tDayManager.dayIsFinished && !tDayManager.editMode : false
				enabled: !tDayManager.timerActive

				onClicked: tDayManager.startWorkout();
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
						value: tDayManager.timerHour
					}
					Rectangle { color : appSettings.fontColor; width: 2; height: 35 }

					TPDigitalClock {
						id: minsClock
						max: 60
						value: tDayManager.timerMinute
					}
					Rectangle { color : appSettings.fontColor; width: 2; height: 35 }

					TPDigitalClock {
						id: secsClock
						max: 60
						value: tDayManager.timerSecond
					}
				}
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")
				flat: false
				visible: btnStartWorkout.visible
				enabled: tDayManager.timerActive

				onClicked: tDayManager.stopWorkout();
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
			visible: tDayManager.dayIsFinished || !tDayManager.mainDateIsToday || tDayManager.editMode

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
			visible: tDayManager.dayIsFinished

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
			visible: tDayManager.splitLetter !== "R"
			enabled: tDayManager.dayIsEditable

			anchors {
				right: parent.right
				rightMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: tDayManager.getExercisesPage(true, trainingDayPage);
		} // bntAddExercise
	} //footer: ToolBar

	SimpleExercisesListPanel {
		id: exercisesPane
		parentPage: trainingDayPage
		onExerciseSelected: exerciseSelectedFromSimpleExercisesList();
		onListClosed: simpleExercisesListClosed();
	}

	function changeComboModel(mesoSplit: string) {
		cboModel.get(0).enabled = mesoSplit.indexOf('A') !== -1;
		cboModel.get(1).enabled = mesoSplit.indexOf('B') !== -1;
		cboModel.get(2).enabled = mesoSplit.indexOf('C') !== -1;
		cboModel.get(3).enabled = mesoSplit.indexOf('D') !== -1;
		cboModel.get(4).enabled = mesoSplit.indexOf('E') !== -1;
		cboModel.get(5).enabled = mesoSplit.indexOf('F') !== -1;
		cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(tDayManager.splitLetter);
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
					tDayManager.changeSplit(cboSplitLetter.valueAt(cboSplitLetter.currentIndex), changeSplitLetterDialog.customBoolProperty1);
				});
				changeSplitLetterDialog.button2Clicked.connect( function() { tDayManager.changeSplit("X"); });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		changeSplitLetterDialog.show(-1);
	}

	property TPBalloonTip msgClearExercises: null
	function showClearExercisesMessage() {
		if (msgClearExercises === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgClearExercises = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Clear exercises list?"),
						message: qsTr("All exercises changes will be removed"), button1Text: qsTr("Yes"), button2Text: qsTr("No"), imageSource: "revert-day.png" } );
					msgClearExercises.button1Clicked.connect(function () { tDayManager.clearExercises(tDayManager); } );
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
	function showAdjustCalendarDialog(newSplitLetter: string) {
		if (!adjustCalendarBox) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				adjustCalendarBox = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Re-adjust meso calendar?"),
					button1Text: qsTr("Adjust"), button2Text: qsTr("Cancel"), customItemSource:"TPDialogWithMessageAndCheckBox.qml",
					customStringProperty1: lblHeader.text, customStringProperty2: qsTr("Only alter this day"), customStringProperty3: "calendar.png" });
				adjustCalendarBox.button1Clicked.connect(function() { tDayManager.adjustCalendar(newSplitLetter, adjustCalendarBox.customBoolProperty1); });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		adjustCalendarBox.show(-1);
	}

	property TPComplexDialog intentDlg: null
	function showIntentionDialog() {
		if (!intentDlg) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				intentDlg = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("What do you want to do today?"),
					button1Text: qsTr("Proceed"), customItemSource:"TPTDayIntentGroup.qml", bClosable: false, customBoolProperty1: bHasMesoPlan,
					customStringProperty2: tDayManager.splitLetter, customModel: previousTDays,
					customBoolProperty2: bHasPreviousTDays, customBoolProperty3: !tDayManager.hasExercises });
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
				tDayManager.loadExercisesFromMesoPlan();
			break;
			case 2: //use previous day
				tDayManager.loadExercisesFromDate(intentDlg.customStringProperty1);
			break;
			case 3: //import from file
				tDayManager.importTrainingDay();
			break;
			case 4: //empty session
				bHasPreviousTDays = false;
				bHasMesoPlan = false;
			break;
		}
	}

	function gotExercise() {
		function readyToProceed(object, id) {
			scrollTimer.init(phantomItem.y);
			return;
		}
		tDayManager.createExerciseObject();
	}

	function placeSetIntoView(ypos: int) {
		scrollTimer.init(ypos);
	}

	function hideSimpleExercisesList() {
		exercisesPane.visible = false;
	}

	function showSimpleExercisesList(multipleSel) {
		exercisesPane.bEnableMultipleSelection = multipleSel;
		exercisesPane.open();
	}

	property TimerDialog dlgSessionLength: null
	function limitedTimeSessionTimer() {
		if (dlgSessionLength === null) {
			var component = Qt.createComponent("qrc:/qml/Dialogs/TimerDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				dlgSessionLength = component.createObject(trainingDayPage, { parentPage: trainingDayPage, timePickerOnly: true,
					windowTitle: qsTr("Length of this training session") });
				dlgSessionLength.onUseTime.connect(function(strtime) { tDayManager.prepareWorkOutTimer(strtime); } );
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		dlgSessionLength.open();
	}

	property TPBalloonTip tipTimeWarn: null
	function displayTimeWarning(timeleft: string, bmin: bool) {
		if (tipTimeWarn === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					tipTimeWarn = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Attention!"),
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

	property TPBalloonTip generalMessage: null
	function showMessageDialog(title: string, message: string, error: bool, msecs: int) {
		if (generalMessage === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					generalMessage = component.createObject(trainingDayPage, { parentPage: trainingDayPage, highlightMessage: true, button1Text: "OK"});
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
	function showRemoveExerciseMessage(exerciseidx: int, exercisename: string) {
		if (msgRemoveExercise === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveExercise = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Remove Exercise?"),
						button1Text: qsTr("Yes"), button2Text: qsTr("No"), imageSource: "remove" } );
					msgRemoveExercise.button1Clicked.connect(function () { tDayManager.removeExercise(exerciseidx); } );
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
	function showRemoveSetMessage(setnumber: int, exerciseidx: int) {
		if (msgRemoveSet === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveSet = component.createObject(trainingDayPage, { parentPage: trainingDayPage, imageSource: "remove",
						message: qsTr("This action cannot be undone."), button1Text: qsTr("Yes"), button2Text: qsTr("No") } );
					msgRemoveSet.button1Clicked.connect(function () { tDayManager.removeSetFromExercise(exerciseidx, setnumber); } );
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
	function resetWorkoutMessage() {
		if (resetWorkoutMsg === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					resetWorkoutMsg = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Reset workout?"),
						message: qsTr("Exercises will not be afected"), button1Text: qsTr("Yes"), button2Text: qsTr("No"), imageSource: "reset.png" } );
					tipTimeWarn.button1Clicked.connect(function () { tDayManager.resetWorkout(); } );
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

	property TPFloatingMenuBar optionsMenu: null
	function showFinishedWorkoutOptions() {
		if (optionsMenu === null) {
			var optionsMenuMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			optionsMenu = optionsMenuMenuComponent.createObject(trainingDayPage, { parentPage: trainingDayPage });
			optionsMenu.addEntry(qsTr("Edit workout"), "edit.png", 0, true);
			optionsMenu.addEntry(qsTr("Reset Workout"), "reset.png", 1, tDayManager.mainDateIsToday);
			optionsMenu.menuEntrySelected.connect(selectedOptionsMenuOption);
		}
		optionsMenu.show(btnFinishedDayOptions, 0);
	}

	function selectedOptionsMenuOption(menuid) {
		switch (menuid) {
			case 0:
				if (tDayManager.editMode)
					tDayManager.setDayIsFinished(true);
				tDayManager.editMode = !tDayManager.editMode;
				tDayManager.dayIsEditable = tDayManager.editMode;
				optionsMenu.setMenuText(0, tDayManager.editMode ? qsTr("Done") : qsTr("Edit workout"));
			break;
			case 1:
				resetWorkoutMessage();
			break;
		}
	}

	readonly property bool bExportEnabled: tDayManager.dayIsFinished && tDayManager.hasExercises
	property TPFloatingMenuBar exportMenu: null
	function showExportMenu() {
		if (exportMenu === null) {
			var exportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			exportMenu = exportMenuComponent.createObject(trainingDayPage, { parentPage: trainingDayPage });
			exportMenu.addEntry(qsTr("Export"), "save-day.png", 0, true);
			if (Qt.platform.os === "android")
				exportMenu.addEntry(qsTr("Share"), "export.png", 1, true);
			exportMenu.menuEntrySelected.connect(function(id) { exportTypeTip.init(id === 1); });
		}
		exportMenu.enableMenuEntry(0, bExportEnabled);
		if (Qt.platform.os === "android")
			exportMenu.enableMenuEntry(1, bExportEnabled);
		exportMenu.show(btnImExport, 0);
	}

	TPBalloonTip {
		id: exportTypeTip
		title: bShare ? qsTr("Share workout?") : qsTr("Export workout to file?")
		imageSource: "export.png"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		parentPage: trainingDayPage

		onButton1Clicked: tDayManager.exportTrainingDay(bShare);

		property bool bShare

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}
} // Page
