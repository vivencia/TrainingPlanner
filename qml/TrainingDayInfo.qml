import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "inexportMethods.js" as INEX
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
	property string mesoName
	property string splitText
	property bool bRealMeso: true
	property var previousTDays: []
	property bool bHasPreviousTDays: false
	property bool bHasMesoPlan: false
	property bool editMode: false

	property bool bAlreadyLoaded

	property date previousDivisionDayDate
	property var btnFloat: null
	property var navButtons: null
	property var timerDialog: null
	property var timerDialogRequester: null
	property var optionsMenu: null

	property bool bEnableMultipleSelection: false
	property bool bShowSimpleExercisesList: false
	property var itemThatRequestedSimpleList: null

	signal mesoCalendarChanged()

	property var splitModel: [ { value:'A', text:'A' }, { value:'B', text:'B' }, { value:'C', text:'C' },
							{ value:'D', text:'D' }, { value:'E', text:'E' }, { value:'F', text:'F' }, { value:'R', text:'R' } ]

	property var imexportMenu: null
	readonly property bool bExportEnabled: tDayModel.dayIsFinished && tDayModel.exerciseCount > 0

	onEditModeChanged: {
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
		cboPreviousTDaysDates.model = previousTDays;
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
		}
	}

	TimePicker {
		id: dlgTimeOut
		hrsDisplay: runCmd.getHourOrMinutesFromStrTime(txtOutTime.text)
		minutesDisplay: runCmd.getMinutesOrSeconsFromStrTime(txtOutTime.text)

		onTimeSet: (hour, minutes) => {
			timeOut = hour + ":" + minutes;
			tDayModel.setTimeOut(timeOut);
			const workoutLenght = runCmd.calculateTimeDifference(timeIn, timeOut);
			updateTimer(workoutLenght.getHours(), workoutLenght.getMinutes(), workoutLenght.getSeconds());
			if (!editMode) {
				itemManager.rollUpExercises();
				appDB.setDayIsFinished(mainDate, true);
			}
		}
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
		title: opt === 0 ? qsTr("Really change split?") : qsTr("Clear exercises list?")
		message: qsTr("All exercises changes will be removed")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		property int opt

		onButton1Clicked: {
			if (opt === 0) {
				appDB.clearExercises();
				changeSplitLetter();
			}
			else
				appDB.clearExercises();

		}

		onButton2Clicked: {
			if (opt === 0) {
				cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(tDayModel.splitLetter() === "" ?
											splitLetter : tDayModel.splitLetter());
			}
		}

		function init(_opt) {
			opt = _opt
			show(-1);
		}
	} //TPBalloonTip

	TPBalloonTip {
		id: tipTimeWarn
		title: qsTr("Attention!")
		message: "<b>" + timeLeft + qsTr("</b> until end of training session!")
		imageSource: "qrc:/images/"+darkIconFolder+"sound-off.png"
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
		imageSource: "qrc:/images/"+darkIconFolder+"time.png"
		button1Text: qsTr("OK")
		highlightMessage: true
	}

	TPBalloonTip {
		id: exportTypeTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"export.png"
		title: bShare ? qsTr("Share workout?") : qsTr("Export workout to file?")
		message: label1.text
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		checkBoxText: qsTr("Human readable?")

		onButton1Clicked: appDB.exportTrainingDay(mainDate, splitLetter, bShare, checkBoxChecked);

		property bool bShare: false

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}

	TPBalloonTip {
		id: resetWorkoutMsg
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"reset.png"
		title: qsTr("Reset workout?");
		message: qsTr("Exercises will not be afected")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")

		onButton1Clicked: itemManager.resetWorkout();
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
				id: label1
				topPadding: 20
				bottomPadding: 20
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				horizontalAlignment: Text.AlignHCenter
				wrapMode: Text.WordWrap
				text: "<b>" + runCmd.formatDate(mainDate) + "</b> : <b>" + mesoName + "</b><br>" +
					qsTr("Workout number: <b>") + tDay + "</b><br>" + "<b>" + splitText + "</b>"
				font.pointSize: AppSettings.fontSizeTitle
				color: AppSettings.fontColor
			}

			GridLayout {
				Layout.fillWidth: true
				columns: 2
				rows: 2
				Layout.leftMargin: 5
				Layout.rightMargin: 5

				Label {
					text: qsTr("Training Division:")
					color: AppSettings.fontColor
					font.pointSize: AppSettings.fontSizeText
					font.bold: true
					Layout.row: 0
					Layout.column: 0
				}

				TPComboBox {
					id: cboSplitLetter
					model: cboModel
					enabled: workoutTimer.active ? false : !tDayModel.dayIsFinished
					Layout.maximumWidth: 100
					Layout.row: 0
					Layout.column: 1

					onActivated: (index) => {
						if (cboModel.get(index).value !== splitLetter) {
							if (exercisesLayout.children.length > 0)
								msgClearExercises.init(0);
							else
								changeSplitLetter();
						}
					}			
				} //TPComboBox

				Label {
					text: qsTr("Training Day #")
					color: AppSettings.fontColor
					font.pointSize: AppSettings.fontSizeText
					font.bold: true
					visible: splitLetter !== 'R'
					Layout.row: 1
					Layout.column: 0
				}
				TPTextInput {
					id: txtTDay
					text: tDay
					width: 50
					visible: splitLetter !== 'R'
					readOnly: true
					Layout.row: 1
					Layout.column: 1

					onTextChanged: tDayModel.setTrainingDay(text);
				} //txtTDay
			} //GridLayout

			Label {
				text: qsTr("Location:")
				color: AppSettings.fontColor
				font.pointSize: AppSettings.fontSizeText
				font.bold: true
				Layout.leftMargin: 5
				visible: splitLetter !== 'R'
			}
			TPTextInput {
				id: txtLocation
				placeholderText: "Academia Golden Era"
				text: tDayModel.location()
				visible: splitLetter !== 'R'
				enabled: !tDayModel.dayIsFinished
				Layout.fillWidth: true
				Layout.rightMargin: 5
				Layout.leftMargin: 5

				onTextChanged: tDayModel.setLocation(text);
			}

			Frame {
				id: frmTrainingTime
				visible: splitLetter !== 'R' && !grpIntent.visible
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
				visible: splitLetter !== 'R'
				color: AppSettings.fontColor
				Layout.leftMargin: 5

				onEditFinished: (new_text) => tDayModel.setDayNotes(new_text);
			}

			TPButton {
				text: qsTr("Use this workout exercises as the default exercises plan for the division ") + splitLetter + qsTr( " of this mesocycle")
				flat: true
				rounded: false
				visible: tDayModel.dayIsFinished && tDayModel.exerciseCount > 0
				width: parent.width - 10
				height: 50
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

					onClicked: msgClearExercises.init(1);
				}
			}

			TPGroupBox {
				id: grpIntent
				text: qsTr("What do you want to do today?")
				Layout.fillWidth: true
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				Layout.bottomMargin: 30
				visible: splitLetter !== "R" && (bHasMesoPlan || bHasPreviousTDays)
				width: parent.width - 20

				property int option
				onOptionChanged: btnChooseIntent.enabled = true;

				ColumnLayout {
					anchors {
						fill: parent
						leftMargin: 5
						rightMargin: 5
					}
					spacing: 0

					TPRadioButton {
						id: optMesoPlan
						text: qsTr("Use the standard exercises plan for the division ") + splitLetter + qsTr(" of the Mesocycle")
						visible: bHasMesoPlan
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						onClicked: grpIntent.option = 1;
					}

					TPRadioButton {
						id: optPreviousDay
						text: qsTr("Base this session off the one from the one the days in the list below")
						visible: bHasPreviousTDays
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						onClicked: grpIntent.option = 2;
					}

					TPComboBox {
						id: cboPreviousTDaysDates
						textRole: ""
						visible: bHasPreviousTDays
						enabled: optPreviousDay.checked
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft
					}

					TPRadioButton {
						id: optEmptySession
						text: qsTr("Start a new session")
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						onClicked: grpIntent.option = 3;
					}

					TPButton {
						id: btnChooseIntent
						text: qsTr("Proceed")
						enabled: false
						Layout.alignment: Qt.AlignCenter

						onClicked: {
							grpIntent.highlight = false;

							switch (grpIntent.option) {
								case 1: //use meso plan
									appDB.loadExercisesFromMesoPlan(splitLetter);
								break;
								case 2: //use previous day
									appDB.loadExercisesFromDate(cboPreviousTDaysDates.currentText);
								break;
								case 3: //empty session
									bHasPreviousTDays = false;
									bHasMesoPlan = false;
								break;
							}
						}
					}
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

	/*Component.onDestruction: {
		if (timerDialog !== null)
			timerDialog.destroy();
		if (navButtons !== null)
			navButtons.destroy();
	}*/

	Keys.onBackPressed: (event) => {
		event.accepted = true;
		if (exercisesPane.visible)
			requestSimpleExercisesList(null, false, false);
		else
			trainingDayPage.StackView.pop();
	}

	Component.onCompleted: {
		mesoName = mesocyclesModel.get(mesoIdx, 1);
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
		visible: !exercisesPane.shown

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
				visible: !tDayModel.dayIsFinished && !editMode && !grpIntent.visible
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

					DigitalClock {
						id: hoursClock
						max: 24
						value: workoutTimer.hours
					}
					Rectangle { color : AppSettings.fontColor; width: 2; height: 35 }

					DigitalClock {
						id: minsClock
						max: 60
						value: workoutTimer.minutes
					}
					Rectangle { color : AppSettings.fontColor; width: 2; height: 35 }

					DigitalClock {
						id: secsClock
						max: 60
						value: workoutTimer.seconds
					}
				}
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")
				visible: !tDayModel.dayIsFinished && !editMode && !grpIntent.visible
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
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"menu.png"
			textUnderIcon: true
			width: 55
			fixedSize: true
			rounded: false
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
			text: qsTr("In/Export")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"import-export.png"
			textUnderIcon: true
			rounded: false
			visible: tDayModel.dayIsFinished
			width: 70
			fixedSize: true

			anchors {
				left: btnFinishedDayOptions.right
				leftMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: INEX.showInExMenu(trainingDayPage, true);
		}

		TPButton {
			id: btnAddExercise
			text: qsTr("Add exercise")
			visible: splitLetter !== 'R'
			enabled: !tDayModel.dayIsFinished ? editMode ? splitLetter !== 'R' : splitLetter !== 'R' && workoutTimer.active : false;
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"exercises-add.png"
			rounded: false
			textUnderIcon: true
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

		onShownChanged: {
			if (navButtons)
				navButtons.visible = !visible;
		}
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

	TPBalloonTip {
		id: adjustCalendarBox
		title: qsTr("Re-adjust meso calendar?")
		checkBoxText: qsTr("Only alter this day")
		imageSource: "qrc:/images/"+darkIconFolder+"calendar.png"
		button1Text: qsTr("Adjust")
		button2Text: qsTr("Cancel")

		property string newSplitLetter

		onButton1Clicked: {
			var bDayIsFinished;
			if (newSplitLetter !== "R") {
				if (splitLetter == "R")
					tDay = parseInt(mesoCalendarModel.getLastTrainingDayBeforeDate(mainDate) + 1);
				bDayIsFinished = tDayModel.dayIsFinished;
			}
			else {
				tDay = 0;
				bDayIsFinished = false;
			}
			if (checkBoxChecked)
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
		var component = Qt.createComponent("FloatingButton.qml", Qt.Asynchronous);
		function finishCreation() {
			btnFloat = component.createObject(trainingDayPage, { text:qsTr("Add set"),
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
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		itemThatRequestedSimpleList = visible ? object : null;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
	}

	function hideSimpleExerciseList() {
		exercisesPane.shown = false;
	}

	function requestTimerDialog(requester, message, mins, secs) {
		if (timerDialog === null) {
			var component = Qt.createComponent("TimerDialog.qml", Qt.Asynchronous);

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
			var component = Qt.createComponent("PageScrollButtons.qml", Qt.Asynchronous);

			function finishCreation() {
				navButtons = component.createObject(trainingDayPage, {});
				navButtons.scrollTo.connect(scrollTraining.setScrollBarPosition);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	function pageActivation() {
		changeComboModel();
		if (navButtons)
			navButtons.visible = true;
	}

	function pageDeActivation() {
		if (navButtons)
			navButtons.visible = false;
		if (btnFloat)
			btnFloat.visible = false;
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
			var optionsMenuMenuComponent = Qt.createComponent("TPFloatingMenuBar.qml");
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
					tDayModel.dayIsFinished = true;
					btnFinishedDayOptions.visible = Qt.binding(function() { return tDayModel.dayIsFinished; });
				}
				editMode = !editMode;
			break;
			case 1:
				resetWorkoutMsg.show(-1);
			break;
		}
	}
} // Page
