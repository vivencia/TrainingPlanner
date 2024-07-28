import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "../"
import "../inexportMethods.js" as INEX
import "../Dialogs"
import "../ExercisesAndSets"
import "../TPWidgets"

import com.vivenciasoftware.qmlcomponents

Page {
	id: trainingDayPage
	objectName: "trainingDayPage"
	width: windowWidth
	height: windowHeight

	required property date mainDate
	required property int mesoId
	required property int mesoIdx
	required property DBTrainingDayModel tDayModel

	property string tDay
	property string splitLetter
	property string timeIn
	property string timeOut

	property string mesoSplit
	property string splitText
	property bool bRealMeso: true
	property var previousTDays: []
	property bool bHasPreviousTDays: false
	property bool bHasMesoPlan: false
	property bool pageOptionsLoaded: false
	property bool editMode: false
	property bool bCalendarChangedPending: false
	property bool bAlreadyLoaded

	property date previousDivisionDayDate
	property var intentionDlg: null
	property var btnFloat: null
	property var navButtons: null
	property var timerDialog: null
	property var timerDialogRequester: null
	property var optionsMenu: null

	property bool bEnableMultipleSelection: false
	property bool bShowSimpleExercisesList: false
	property var itemThatRequestedSimpleList: null

	property bool intentDialogShown: splitLetter !== "R" && (bHasMesoPlan || bHasPreviousTDays || tDayModel.exerciseCount === 0)

	onPageOptionsLoadedChanged: {
		if (pageOptionsLoaded && intentDialogShown)
			showIntentionDialog();
	}

	signal mesoCalendarChanged()
	signal pageActivated();
	signal pageDeActivated();

	property var splitModel: [ { value:'A', text:'A' }, { value:'B', text:'B' }, { value:'C', text:'C' },
							{ value:'D', text:'D' }, { value:'E', text:'E' }, { value:'F', text:'F' }, { value:'R', text:'R' } ]

	property var imexportMenu: null
	readonly property bool bExportEnabled: tDayModel.dayIsFinished && tDayModel.exerciseCount > 0

	onEditModeChanged: {
		if (optionsMenu !== null)
			optionsMenu.setMenuText(0, editMode ? qsTr("Done") : qsTr("Edit workout"));
	}

	onBExportEnabledChanged: {
		if (imexportMenu) {
			imexportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				imexportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	onPreviousTDaysChanged: {
		if (intentionDlg)
			intentionDlg.customModel = previousTDays;
	}

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

	ListModel {
		id: cboModel
	}

	TimePicker {
		id: dlgTimeIn
		hrsDisplay: runCmd.getHourOrMinutesFromStrTime(txtInTime.text)
		minutesDisplay: runCmd.getMinutesOrSeconsFromStrTime(txtInTime.text)

		onTimeSet: (hour, minutes) => {
			timeIn = hour + ":" + minutes;
			tDayModel.setTimeIn(timeIn);
			if (timeOut != "--:--")
				timeManuallyEdited();
		}
	}
	TimePicker {
		id: dlgTimeOut
		hrsDisplay: runCmd.getHourOrMinutesFromStrTime(txtOutTime.text)
		minutesDisplay: runCmd.getMinutesOrSeconsFromStrTime(txtOutTime.text)

		onTimeSet: (hour, minutes) => {
			timeOut = hour + ":" + minutes;
			tDayModel.setTimeOut(timeOut);
			if (timeIn != "--:--")
				timeManuallyEdited();
		}
	}
	function timeManuallyEdited() {
		const workoutLenght = runCmd.calculateTimeDifference(timeIn, timeOut);
		updateTimer(workoutLenght.getHours(), workoutLenght.getMinutes(), workoutLenght.getSeconds());
		appDB.setDayIsFinished(mainDate, true);
		itemManager.rollUpExercises();
	}

	TimerDialog {
		id: dlgSessionLength
		timePickerOnly: true
		windowTitle: qsTr("Length of this training session")

		onUseTime: (strtime) => {
			workoutTimer.stopWatch = false;
			workoutTimer.prepareTimer(strtime + ":00");
		}
	}

	TimePicker {
		id: dlgTimeEndSession
		hrsDisplay: runCmd.getHourFromCurrentTime()
		minutesDisplay: runCmd.getMinutesFromCurrentTime()
		bOnlyFutureTime: true

		onTimeSet: (hour, minutes) => {
			workoutTimer.stopWatch = false;
			workoutTimer.prepareTimer(runCmd.calculateTimeDifference_str(
					runCmd.getCurrentTimeString(), hour + ":" + minutes));
		}
	}

	TPTimer {
		id: workoutTimer
		alarmSoundFile: "qrc:/sounds/timer-end.wav"
		stopWatch: true
		interval: 1000

		Component.onCompleted: {
			setRunCommandsObject(runCmd);
			addWarningAtMinute(15);
			addWarningAtMinute(5);
			addWarningAtMinute(1);
		}
	}

	TPBalloonTip {
		id: msgClearExercises
		title: qsTr("Clear exercises list?")
		message: qsTr("All exercises changes will be removed")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "revert-day.png"

		onButton1Clicked: appDB.clearExercises();
	} //TPBalloonTip

	TPBalloonTip {
		id: tipTimeWarn
		title: qsTr("Attention!")
		message: "<b>" + timeLeft + qsTr("</b> until end of training session!")
		imageSource: "sound-off.png"
		button1Text: qsTr("OK")

		property string timeLeft
		onButton1Clicked: workoutTimer.stopAlarmSound();
	}
	function displayTimeWarning(timeleft: string, bmin: bool) {
		tipTimeWarn.timeLeft = timeleft + (bmin ? qsTr(" minutes") : qsTr(" seconds"));
		var timeout;
		if (!bmin)
		{
			timeout = 60000;
			workoutTimer.setAlarmSoundLoops(4);
		}
		else
			timeout = 18000;
		tipTimeWarn.showTimed(timeout, 0);
	}

	TPBalloonTip {
		id: timerDlgMessage
		title: qsTr("Attention!")
		message: qsTr("Only one timer window can be opened at a time!")
		imageSource: "time.png"
		button1Text: qsTr("OK")
		highlightMessage: true
	}

	TPComplexDialog {
		id: exportTypeTip
		title: bShare ? qsTr("Share workout?") : qsTr("Export workout to file?")
		customStringProperty1: lblHeader.text
		customStringProperty2: qsTr("Human readable?")
		customStringProperty3: "export.png"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		customItemSource: "TPDialogWithMessageAndCheckBox.qml"
		parentPage: trainingDayPage

		onButton1Clicked: appDB.exportTrainingDay(mainDate, splitLetter, bShare, checkBoxChecked);

		property bool bShare: false

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}

	TPBalloonTip {
		id: resetWorkoutMsg
		imageSource: "reset.png"
		title: qsTr("Reset workout?");
		message: qsTr("Exercises will not be afected")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")

		onButton1Clicked: itemManager.resetWorkout();
	}

	TPBalloonTip {
		id: calendarChangedWarning
		imageSource: "warning.png"
		title: qsTr("Calendar changed! Update?")
		message: qsTr("Training division: ") + splitLetter + " -> " + newSplitLetter + qsTr("\nWorkout number: ") + tDay + " -> " + newtDay
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")

		onButton1Clicked: acceptChanges();

		function acceptChanges() {
			bCalendarChangedPending = false;
			splitLetter = newSplitLetter;
			tDay = newtDay;
			splitText = newSplitText;
			tDayModel.setTrainingDay(tDay);
			tDayModel.setSplitLetter(splitLetter);
			msgClearExercises.show(-1);
			saveWorkout();
		}

		property string newSplitLetter
		property string newtDay
		property string newSplitText
	}
	function warnCalendarChanged(newsplitletter: string, newtday: string, newsplittext: string) {
		bCalendarChangedPending = true;
		calendarChangedWarning.newSplitLetter = newsplitletter;
		calendarChangedWarning.newtDay = newtday;
		calendarChangedWarning.newSplitText = newsplittext;
		calendarChangedWarning.show(-1);
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
				topMargin: 5
			}

			Label {
				id: lblHeader
				topPadding: 20
				bottomPadding: 20
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				horizontalAlignment: Text.AlignHCenter
				wrapMode: Text.WordWrap
				text: "<b>" + runCmd.formatDate(mainDate) + "</b> : <b>" + mesocyclesModel.get(mesoIdx, 1) + "</b><br>" +
					(splitLetter !== "R" ? (qsTr("Workout number: <b>") + tDay + "</b><br>" + "<b>" + splitText + "</b>") :
										qsTr("Rest day"))
				font.pointSize: AppSettings.fontSizeTitle
				color: AppSettings.fontColor
			}

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5

				Label {
					text: qsTr("Training Division:")
					color: AppSettings.fontColor
					font.pointSize: AppSettings.fontSizeText
					font.bold: true
				}

				TPComboBox {
					id: cboSplitLetter
					model: cboModel
					enabled: workoutTimer.active ? false : !tDayModel.dayIsFinished
					Layout.maximumWidth: 100

					onActivated: (index) => {
						if (cboModel.get(index).value !== splitLetter) {
							if (exercisesLayout.children.length > 0)
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

				Label {
					text: qsTr("Location:")
					color: AppSettings.fontColor
					font.pointSize: AppSettings.fontSizeText
					font.bold: true
					visible: txtLocation.visible
					Layout.row: 1
					Layout.column: 0
				}
				TPTextInput {
					id: txtLocation
					placeholderText: "Academia Golden Era"
					text: tDayModel.location()
					visible: splitLetter != "R"
					enabled: !tDayModel.dayIsFinished
					Layout.row: 1
					Layout.column: 1
					Layout.fillWidth: true

					onTextChanged: tDayModel.setLocation(text);
				}
			}

			Frame {
				id: frmTrainingTime
				visible: splitLetter !== 'R' && !intentDialogShown
				enabled: !tDayModel.dayIsFinished && !workoutTimer.active
				height: 330
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5

				background: Rectangle {
					border.color: AppSettings.fontColor
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
								workoutTimer.prepareTimer("");
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
							onClicked: dlgSessionLength.open();
						}
						TPButton {
							id: btnTimeHour
							text: qsTr("By time of day")
							visible: optTimeConstrainedSession.checked
							Layout.alignment: Qt.AlignCenter
							onClicked: dlgTimeEndSession.open();
						}
					} //RowLayout

					Label {
						id: lblInTime
						color: AppSettings.fontColor
						font.pointSize: AppSettings.fontSizeText
						font.bold: true
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

						TPRoundButton {
							id: btnInTime
							width: 40
							height: 40
							imageName: "time.png"

							anchors {
								top: parent.top
								topMargin: -15
								left: txtInTime.right
							}

							onClicked: dlgTimeIn.open();
						}
					} //Label

					Label {
						id: lblOutTime
						color: AppSettings.fontColor
						font.pointSize: AppSettings.fontSizeText
						font.bold: true
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

						TPRoundButton {
							id: btnOutTime
							width: 40
							height: 40
							imageName: "time.png"

							anchors {
								top: parent.top
								topMargin: -15
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
				readOnly: !tDayModel.dayIsFinished
				visible: splitLetter != "R"
				color: AppSettings.fontColor
				Layout.leftMargin: 5

				onEditFinished: (new_text) => tDayModel.setDayNotes(new_text);
			}

			TPButton {
				text: qsTr("Use this workout exercises as the default exercises plan for the division ") + splitLetter + qsTr( " of this mesocycle")
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
					appDB.convertTDayToPlan(tDayModel);
					enabled = editMode;
				}
			}

			Label {
				id: lblExercisesStart
				text: qsTr("--- EXERCISES ---")
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				color: AppSettings.fontColor
				font.weight: Font.Black
				font.pointSize: AppSettings.fontSizeTitle
				visible: splitLetter !== 'R'
				height: 40
				Layout.bottomMargin: 10
				Layout.fillWidth: true
				Layout.minimumHeight: height
				Layout.maximumHeight: height

				background: Rectangle {
					gradient: Gradient {
						orientation: Gradient.Horizontal
						GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
						GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
						GradientStop { position: 0.50; color: AppSettings.primaryColor; }
						GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
					}
					opacity: 0.8
				}

				TPRoundButton {
					id: btnClearExercises
					width: 40
					height: 40
					enabled: !tDayModel.dayIsFinished
					visible: exercisesLayout.children.length > 0
					ToolTip.text: "Remove all exercises"
					imageName: "revert-day.png"

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
						leftMargin: 5
					}

					onClicked: msgClearExercises.show(-1);
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
			navButtons.visible = true;
		}

		function setScrollBarPosition(pos) {
			if (pos === 0)
				ScrollBar.vertical.setPosition(0);
			else
				ScrollBar.vertical.setPosition(pos - ScrollBar.vertical.size/2);
		}
	} // ScrollView scrollTraining

	Component.onDestruction: {
		if (bCalendarChangedPending)
			calendarChangedWarning.acceptChanges();
	}

	Component.onCompleted: {
		mesoSplit = mesocyclesModel.get(mesoIdx, 6);
		bRealMeso = mesocyclesModel.get(mesoIdx, 3) !== "0";
		trainingDayPage.StackView.activating.connect(pageActivation);
		trainingDayPage.StackView.onDeactivating.connect(pageDeActivation);
		tDayModel.saveWorkout.connect(saveWorkout);
	}

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
		if (cboModel.count > 0)
			cboModel.clear();
		if (mesoSplit.indexOf('A') !== -1)
			cboModel.append(splitModel[0]);
		if (mesoSplit.indexOf('B') !== -1)
			cboModel.append(splitModel[1]);
		if (mesoSplit.indexOf('C') !== -1)
			cboModel.append(splitModel[2]);
		if (mesoSplit.indexOf('D') !== -1)
			cboModel.append(splitModel[3]);
		if (mesoSplit.indexOf('E') !== -1)
			cboModel.append(splitModel[4]);
		if (mesoSplit.indexOf('F') !== -1)
			cboModel.append(splitModel[5]);
		if (mesoSplit.indexOf('R') !== -1)
			cboModel.append(splitModel[6]);

		cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(tDayModel.splitLetter() === "" ? splitLetter : tDayModel.splitLetter());
	}

	footer: ToolBar {
		id: dayInfoToolBar
		width: parent.width
		height: 100
		visible: !exercisesPane.visible

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		RowLayout {
			id: workoutLengthRow
			height: 50
			spacing: 5
			visible: splitLetter !== 'R'
			anchors {
				left: parent.left
				leftMargin: 5
				top: parent.top
				right: parent.right
				rightMargin: 5
			}

			Label {
				text: !tDayModel.dayIsFinished ? qsTr("Workout:") : qsTr("Workout session length: ")
				color: AppSettings.fontColor
				font.bold: true
				font.pointSize: AppSettings.fontSizeText
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				flat: false
				visible: !tDayModel.dayIsFinished && !editMode && !intentDialogShown
				enabled: !workoutTimer.active

				onClicked: {
					workoutTimer.startTimer();
					exercisesLayout.enabled = true;
					timeIn = runCmd.getCurrentTimeString();
					tDayModel.setTimeIn(timeIn);
					workoutTimer.timeWarning.connect(displayTimeWarning);
				}
			}

			Rectangle {
				id: workoutLengthClock
				antialiasing: true
				width: spinnerLayout.width
				height: 35
				color: AppSettings.primaryColor

				RowLayout {
					id: spinnerLayout
					spacing: 2

					TPDigitalClock {
						id: hoursClock
						max: 24
						value: workoutTimer.hours
					}
					Rectangle { color : AppSettings.fontColor; width: 2; height: 35 }

					TPDigitalClock {
						id: minsClock
						max: 60
						value: workoutTimer.minutes
					}
					Rectangle { color : AppSettings.fontColor; width: 2; height: 35 }

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
				visible: !tDayModel.dayIsFinished && !editMode && !intentDialogShown
				enabled: workoutTimer.active

				onClicked: {
					workoutTimer.stopTimer();
					const sessionLength = workoutTimer.elapsedTime();
					updateTimer(sessionLength.getHours(), sessionLength.getMinutes(), sessionLength.getSeconds());
					timeOut = runCmd.getCurrentTimeString();
					tDayModel.setTimeOut(timeOut);
					itemManager.rollUpExercises();
					appDB.setDayIsFinished(mainDate, true);
				}
			}
		}

		TPButton {
			id: btnFinishedDayOptions
			imageSource: "menu.png"
			rounded: false
			flat: false
			fixedSize: true
			width: 55
			height: 55
			visible: tDayModel.dayIsFinished

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
			id: btnImExport
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

			onClicked: INEX.showInExMenu(trainingDayPage, false);
		}

		TPButton {
			id: btnAddExercise
			text: qsTr("Add exercise")
			imageSource: "exercises-add.png"
			rounded: false
			textUnderIcon: true
			flat: false
			height: 55
			visible: splitLetter !== 'R'
			enabled: !tDayModel.dayIsFinished ? editMode ? splitLetter !== 'R' : splitLetter !== 'R' && workoutTimer.active : false;

			anchors {
				right: parent.right
				rightMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			property bool bFirstClick: true

			onClicked: {
				if (navButtons)
					navButtons.visible = false;
				if (btnFloat)
					btnFloat.visible = false;
				appDB.openExercisesListPage(true, bFirstClick ? trainingDayPage : null);
				bFirstClick = false;
			}
		} // bntAddExercise
	} //footer: ToolBar

	SimpleExercisesListPanel {
		id: exercisesPane
		onVisibleChanged: navButtons.visible = !visible;
	}

	onSplitLetterChanged: {
		switch (splitLetter) {
			case 'A': splitText = mesoSplitModel.get(mesoIdx, 2); break;
			case 'B': splitText = mesoSplitModel.get(mesoIdx, 3); break;
			case 'C': splitText = mesoSplitModel.get(mesoIdx, 4); break;
			case 'D': splitText = mesoSplitModel.get(mesoIdx, 5); break;
			case 'E': splitText = mesoSplitModel.get(mesoIdx, 6); break;
			case 'F': splitText = mesoSplitModel.get(mesoIdx, 7); break;
			default: return;
		}
		exercisesListModel.makeFilterString(splitText);
	}

	function saveWorkout() {
		appDB.saveTrainingDay();
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

		onButton1Clicked: {
			var bDayIsFinished;
			if (newSplitLetter !== "R") {
				if (splitLetter == "R")
					tDay = appDB.getWorkoutNumberForTrainingDay(mainDate);
				bDayIsFinished = tDayModel.dayIsFinished;
			}
			else {
				tDay = 0;
				bDayIsFinished = false;
			}
			if (customBoolProperty1)
				appDB.updateMesoCalendarEntry(mainDate, tDay, newSplitLetter, bDayIsFinished);
			else
				appDB.updateMesoCalendarModel(mesoSplit, mainDate, newSplitLetter);
			splitLetter = newSplitLetter;
			tDayModel.setTrainingDay(tDay);
			tDayModel.setSplitLetter(splitLetter);
			saveWorkout();
			if (splitLetter !== "R")
				appDB.verifyTDayOptions(mainDate, splitLetter);
		}

		onButton2Clicked:
			cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(splitLetter);
	}

	function changeSplitLetter() {
		if (bRealMeso && cboSplitLetter.currentValue !== splitLetter) {
			adjustCalendarBox.newSplitLetter = cboSplitLetter.currentValue;
			adjustCalendarBox.show(-1);
		}
	}

	function showIntentionDialog() {
		if (!intentionDlg) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				intentionDlg = component.createObject(mainwindow, { parentPage: trainingDayPage, title:qsTr("What do you want to do today?"), button1Text: qsTr("Proceed"),
						customItemSource:"TPTDayIntentGroup.qml", bClosable: false, customBoolProperty1: bHasMesoPlan,
						customBoolProperty2: bHasPreviousTDays, customBoolProperty3: tDayModel.exerciseCount === 0 });
				intentionDlg.button1Clicked.connect(intentChosen);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		intentionDlg.show(-1);
	}

	function intentChosen() {
		switch (intentionDlg.customIntProperty1) {
			case 1: //use meso plan
				appDB.loadExercisesFromMesoPlan(splitLetter);
			break;
			case 2: //use previous day
				appDB.loadExercisesFromDate(intentionDlg.customStringProperty1);
			break;
			case 3: //import from file
				mainwindow.chooseFileToImport();
			break;
			case 4: //empty session
				bHasPreviousTDays = false;
				bHasMesoPlan = false;
			break;
		}
		intentDialogShown = Qt.binding(function() { return splitLetter !== "R" && (bHasMesoPlan || bHasPreviousTDays); });
	}

	function gotExercise() {
		function readyToProceed(object, id) {
			appDB.getItem.disconnect(readyToProceed);
			createNavButtons();
			scrollTimer.init(phantomItem.y);
			return;
		}

		appDB.getItem.connect(readyToProceed);
		itemManager.createExerciseObject(exercisesListModel);
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
		itemThatRequestedSimpleList = visible ? object : null;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
	}

	function requestTimerDialog(requester, message, mins, secs) {
		if (timerDialog === null) {
			var component = Qt.createComponent("qrc:/qml/Dialogs/TimerDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				timerDialog = component.createObject(trainingDayPage, { bJustMinsAndSecs:true, simpleTimer:false });
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
			timerDlgMessage.close();
			timerDialog.open();
		}
		else
			timerDlgMessage.showTimed(5000, 0);
	}

	function timerDialogUseButtonClicked(strTime) {
		timerDialogRequester.timeChanged(strTime);
	}

	function timerDialogClosed() {
		timerDialogRequester = null;
		timerDlgMessage.close();
	}

	function createNavButtons() {
		if (navButtons === null) {
			var component = Qt.createComponent("qrc:/qml/ExercisesAndSets/PageScrollButtons.qml", Qt.Asynchronous);

			function finishCreation() {
				navButtons = component.createObject(trainingDayPage, { parentPage: trainingDayPage });
				navButtons.scrollTo.connect(scrollTraining.setScrollBarPosition);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	function pageActivation() {
		itemManager.setCurrenttDay(mainDate);
		changeComboModel();
		pageActivated();
	}

	function pageDeActivation() {
		pageDeActivated();
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

	function showFinishedWorkoutOptions() {
		if (optionsMenu === null) {
			var optionsMenuMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			optionsMenu = optionsMenuMenuComponent.createObject(trainingDayPage, {});
			optionsMenu.addEntry(qsTr("Edit workout"), "edit.png", 0);
			optionsMenu.addEntry(qsTr("Reset Workout"), "reset.png", 1);
			optionsMenu.menuEntrySelected.connect(selectedOptionsMenuOption);
		}
		optionsMenu.show(btnFinishedDayOptions, 0);
	}

	function selectedOptionsMenuOption(menuid) {
		switch (menuid) {
			case 0:
				if (!editMode) {
					tDayModel.dayIsFinished = false;
					exercisesLayout.enabled = true;
					btnFinishedDayOptions.visible = true;
				}
				else {
					itemManager.rollUpExercises();
					appDB.setDayIsFinished(mainDate, true);
					btnFinishedDayOptions.visible = Qt.binding(function() { return tDayModel.dayIsFinished; });
				}
				editMode = !editMode;
			break;
			case 1:
				resetWorkoutMsg.show(-1);
			break;
		}
	}

	property var changeSplitLetterDialog: null
	function showSplitLetterChangedDialog() {
		if (!changeSplitLetterDialog) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				changeSplitLetterDialog = component.createObject(mainwindow, { parentPage: trainingDayPage, button1Text: qsTr("Yes"),
					button2Text: qsTr("No"), customStringProperty1: qsTr("Really change split?"), customStringProperty2: qsTr("Clear exercises list?"),
					customStringProperty3: "remove.png", customItemSource:"TPDialogWithMessageAndCheckBox.qml" });
				changeSplitLetterDialog.button1Clicked.connect( function() {
					if (changeSplitLetterDialog.customBoolProperty1)
						appDB.clearExercises();
					changeSplitLetter();
				} );
				changeSplitLetterDialog.button2Clicked.connect( function() {
					cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(tDayModel.splitLetter() === "" ?
													splitLetter : tDayModel.splitLetter())
				} );
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		changeSplitLetterDialog.show(-1);
	}
} // Page
