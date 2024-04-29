import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtMultimedia

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
	property bool bDayIsFinished

	property date sessionStart

	property bool bFirstTime: false
	property bool bAlreadyLoaded
	property bool bStopBounce: false

	property date previousDivisionDayDate
	property int scrollBarPosition: 0
	property var btnFloat: null
	property var navButtons: null
	property var firstTimeTip: null
	property var timerDialog: null
	property var timerDialogRequester: null

	property bool bEnableMultipleSelection: false
	property bool bShowSimpleExercisesList: false
	property var itemThatRequestedSimpleList: null

	signal mesoCalendarChanged()

	property var splitModel: [ { value:'A', text:'A' }, { value:'B', text:'B' }, { value:'C', text:'C' },
							{ value:'D', text:'D' }, { value:'E', text:'E' }, { value:'F', text:'F' }, { value:'R', text:'R' } ]

	onPreviousTDaysChanged: {
		cboPreviousTDaysDates.model = previousTDays;
		bHasPreviousTDays = previousTDays.length > 0;
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
			bDayIsFinished = true;
		}
	}

	TimerDialog {
		id: dlgSessionLength
		timePickerOnly: true
		windowTitle: qsTr("Length of this training session")

		onUseTime: (strtime) => runCmd.prepareWorkoutTimer(strtime);
	}

	TimePicker {
		id: dlgTimeEndSession
		hrsDisplay: runCmd.getCurrentTimeString()
		minutesDisplay: runCmd.getCurrentTimeString()

		onTimeSet: (hour, minutes) => runCmd.prepareWorkoutTimer(hour + ":" + minutes);
	}

	SoundEffect {
		id: playSound
		source: "qrc:/sounds/timer-end.wav"
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
			itemManager.clearExercises();
			if (opt === 0)
				changeSplitLetter();
		}

		onButton2Clicked: {
			if (opt === 0) {
				cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(tDayModel.splitLetter() === "" ?
											splitLetter : tDayModel.splitLetter());
			}
		}

		function init(_opt) {
			opt = _opt
			show(windowHeight / 2);
		}
	} //TPBalloonTip

	TPBalloonTip {
		id: tipTimeWarn
		title: qsTr("Attention!")
		message: "<b>" + timeLeft + qsTr("</b> until end of training session!")
		imageSource: "qrc:/images/"+darkIconFolder+"sound-off.png"
		button1Text: qsTr("OK")

		property string timeLeft
		onButton1Clicked: playSound.stop();
	}
	function displayTimeWarning(timeleft: string, bmin: bool) {
		tipTimeWarn.timeLeft = timeleft + (bmin ? qsTr("minutes") : qsTr("seconds"));
		var timeout;
		if (!bmin)
		{
			timeout = 60000;
			playSound.loops = 4;
		}
		else
			timeout = 18000;
		playSound.play();
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

	ScrollView {
		id: scrollTraining
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		contentWidth: trainingDayPage.width //stops bouncing to the sides
		contentHeight: colMain.height + colExercises.implicitHeight
		anchors.fill: parent

		ScrollBar.vertical.onPositionChanged: {
			if (bStopBounce) {
				if (ScrollBar.vertical.position < scrollBarPosition) {
					scrollToPos(scrollBarPosition);
				}
			}
			else {
				if (navButtons) {
					//console.log(ScrollBar.vertical.position);
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
		}

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
				Layout.maximumWidth: parent.width - 10
				Layout.leftMargin: 5
				horizontalAlignment: Qt.AlignHCenter
				wrapMode: Text.WordWrap
				text: "<b>" + runCmd.formatDate(mainDate) + "</b> : <b>" + mesoName + "</b><br>" + qsTr("Trainning: <b>") + splitText + "</b>"
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
					enabled: model.count > 0
					Layout.maximumWidth: 100
					Layout.row: 0
					Layout.column: 1

					onActivated: (index) => {
						if (cboModel.get(index).value !== splitLetter) {
							if (colExercises.children.length > 0)
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

			TPCheckBox {
				id: chkAdjustCalendar
				text: qsTr("Re-adjust meso calendar from this day?")
				checked: false
				visible: false
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				Layout.fillWidth: true
			} //TPCheckBox

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
				Layout.fillWidth: true
				Layout.rightMargin: 10
				Layout.leftMargin: 5

				onTextChanged: tDayModel.setLocation(text);
			}

			Frame {
				id: frmTrainingTime
				visible: splitLetter !== 'R'
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 10
				height: 330

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
						enabled: !runCmd.timerRunning
						Layout.fillWidth: true

						onClicked: {
							if (checked)
								runCmd.prepareWorkoutTimer();
						}
					}
					TPRadioButton {
						id: optTimeConstrainedSession
						text: qsTr("Time constrained session")
						checked: false
						enabled: !runCmd.timerRunning
						Layout.fillWidth: true
					}

					RowLayout {
						Layout.fillWidth: true
						Layout.leftMargin: 30
						Layout.bottomMargin: 10

						TPButton {
							id: btnTimeLength
							text: qsTr("By duration")
							enabled: !bDayIsFinished && !runCmd.timerRunning
							visible: optTimeConstrainedSession.checked
							Layout.alignment: Qt.AlignCenter
							onClicked: dlgSessionLength.open();
						}
						TPButton {
							id: btnTimeHour
							text: qsTr("By time of day")
							enabled: !bDayIsFinished && !runCmd.timerRunning
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

						RoundButton {
							id: btnInTime
							width: 40
							height: 40
							enabled: !runCmd.timerRunning
							anchors {
								top: parent.top
								topMargin: -15
								left: txtInTime.right
							}

							Image {
								source: "qrc:/images/"+darkIconFolder+"time.png"
								fillMode: Image.PreserveAspectFit
								asynchronous: true
								width: 20
								height: 20
								anchors {
									verticalCenter: parent.verticalCenter
									horizontalCenter: parent.horizontalCenter
								}
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

						RoundButton {
							id: btnOutTime
							enabled: !runCmd.timerRunning
							width: 40
							height: 40

							anchors {
								top: parent.top
								topMargin: -15
								left: txtOutTime.right
							}

							Image {
								source: "qrc:/images/"+darkIconFolder+"time.png"
								fillMode: Image.PreserveAspectFit
								asynchronous: true
								width: 20
								height: 20
								anchors {
									verticalCenter: parent.verticalCenter
									horizontalCenter: parent.horizontalCenter
								}
							}

							onClicked: dlgTimeOut.open();
						}
					} // RowLayout
				} //ColumnLayout
			} //Frame

			SetNotesField {
				info: qsTr("This training session considerations:")
				text: tDayModel.dayNotes()
				visible: splitLetter !== 'R'
				color: AppSettings.fontColor
				Layout.leftMargin: 5

				onEditFinished: (new_text) => tDayModel.setDayNotes(new_text);
			}

			Frame {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 20
				visible: bDayIsFinished && tDayModel.exerciseCount > 0

				background: Rectangle {
					border.color: AppSettings.fontColor
					color: "transparent"
					radius: 6
				}

				ColumnLayout {
					anchors.fill: parent
					spacing: 0

					Label {
						text: qsTr("Replace exercises plan for this division with this day's training list?")
						wrapMode: Text.WordWrap
						font.pointSize: AppSettings.fontSizeText
						font.bold: true
						Layout.topMargin: 20
						Layout.leftMargin: 5
						color: AppSettings.fontColor
						Layout.fillWidth: true
						width: parent.width - 5
						Layout.bottomMargin: 2
						padding: 0
					}

					TPButton {
						id: btnConvertToExercisePlanner
						text: qsTr("Go")
						Layout.alignment: Qt.AlignCenter
						Layout.bottomMargin: 20
						onClicked: appDB.convertTDayToPlan(tDayModel);
					}
				}
			} //Frame

			Label {
				id: lblExercisesStart
				text: qsTr("--- EXERCISES ---")
				color: AppSettings.fontColor
				font.weight: Font.Black
				font.pointSize: AppSettings.fontSizeTitle
				visible: splitLetter !== 'R'
				Layout.alignment: Qt.AlignCenter
				Layout.bottomMargin: 2

				RoundButton {
					id: btnClearExercises
					anchors.right: parent.left
					anchors.verticalCenter: parent.verticalCenter
					anchors.rightMargin: 5
					width: 40
					height: 40
					visible: colExercises.children.length > 0
					ToolTip.text: "Remove all exercises"

					Image {
						source: "qrc:/images/"+darkIconFolder+"revert-day.png"
						width: 20
						height: 20
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
					}

					onClicked: msgClearExercises.init(1);
				}
			}

			TPGroupBox {
				id: grpIntent
				text: qsTr("What do you want to do today?")
				Layout.fillWidth: true
				Layout.rightMargin: 10
				Layout.leftMargin: 5
				Layout.bottomMargin: 30
				visible: bHasMesoPlan || bHasPreviousTDays
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
						text: qsTr("Begin")
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
									placeTipOnAddExercise();
								break;
							}
						}
					}
				}
			}
		}// colMain

		GridLayout {
			id: colExercises
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

		function scrollToPos(y_pos) {
			contentItem.contentY = y_pos;
			navButtons.visible = true;
		}

		function setScrollBarPosition2(pos) {
			ScrollBar.vertical.setPosition(pos);
		}

		function setScrollBarPosition(pos) {
			if (pos === 0)
				ScrollBar.vertical.setPosition(0);
			else
				ScrollBar.vertical.setPosition(pos - ScrollBar.vertical.size/2);
		}
	} // ScrollView scrollTraining

	Component.onDestruction: {
		if (timerDialog !== null)
			timerDialog.destroy();
		if (navButtons !== null)
			navButtons.destroy();
	}

	Component.onCompleted: {
		mesoName = mesocyclesModel.get(mesoIdx, 1);
		mesoSplit = mesocyclesModel.get(mesoIdx, 6);
		bRealMeso = mesocyclesModel.get(mesoIdx, 3) !== "0";
		trainingDayPage.StackView.activating.connect(pageActivation);
		trainingDayPage.StackView.onDeactivating.connect(pageDeActivation);
	}

	Timer {
		id: bounceTimer
		interval: 200
		running: false
		repeat: false

		onTriggered: {
			bStopBounce = false;
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
			anchors {
				left: parent.left
				leftMargin: 5
				top: parent.top
				right: parent.right
				rightMargin: 5
			}

			Label {
				text: !bDayIsFinished ? qsTr("Workout:") : qsTr("Workout session length: ")
				color: AppSettings.fontColor
				font.bold: true
				font.pointSize: AppSettings.fontSizeText
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				visible: !bDayIsFinished

				onClicked: {
					runCmd.workoutTimerTriggered.connect(updateTimer);
					runCmd.startWorkoutTimer();
					timeIn = runCmd.getCurrentTimeString();
					tDayModel.setTimeIn(timeIn);
					runCmd.timeWarning.connect(displayTimeWarning);
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
					}
					Rectangle { color : AppSettings.fontColor; width: 2; height: 35 }

					DigitalClock {
						id: minsClock
						max: 60
					}
					Rectangle { color : AppSettings.fontColor; width: 2; height: 35 }

					DigitalClock {
						id: secsClock;
						max: 60;
					}
				}
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")
				visible: !bDayIsFinished

				onClicked: {
					runCmd.stopWorkoutTimer();
					timeOut = runCmd.getCurrentTimeString();
					tDayModel.setTimeOut(timeOut);
					bDayIsFinished = true;
					runCmd.workoutTimerTriggered.disconnect(updateTimer);
					runCmd.timeWarning.disconnect(displayTimeWarning);
				}
			}
		}

		TPButton {
			id: btnSaveDay
			enabled: tDayModel.modified
			text: qsTr("Log Workout")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"save-day.png"
			textUnderIcon: true
			anchors {
				left: parent.left
				leftMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: {
				if (tDayModel.id() === -1) {
					var id;
					function continueSave(_id) {
						if (_id === id) {
							appDB.databaseReady.disconnect(continueSave);
							appDB.updateTrainingDayExercises();
						}
					}
					id = appDB.pass_object(tDayModel);
					appDB.databaseReady.connect(continueSave);
					appDB.newTrainingDay();
				}
				else {
					appDB.pass_object(tDayModel);
					appDB.updateTrainingDay();
					appDB.updateTrainingDayExercises();
				}
				if (bRealMeso && chkAdjustCalendar.visible)
				{
					if (!chkAdjustCalendar.checked)
						appDB.updateMesoCalendarEntry(mainDate, tDay, splitLetter);
					else
						appDB.updateMesoCalendarModel(mesoSplit, mainDate, splitLetter, tDay);
					chkAdjustCalendar.visible = false;
				}
			}
		} //btnSaveDay

		TPButton {
			id: btnAddExercise
			text: qsTr("Add exercise")
			enabled: splitLetter !== 'R' && !grpIntent.visible
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"exercises-add.png"
			textUnderIcon: true
			anchors {
				right: parent.right
				rightMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: {
				if (navButtons !== null)
					navButtons.hideButtons();
				if (btnFloat)
					btnFloat.visible = false;

				function openTDayExercisesPage(object, id) {
					appDB.getPage.disconnect(openTDayExercisesPage);
					if (id === 999) //999 first time creation
						object.exerciseChosen.connect(gotExercise);
					object.bChooseButtonEnabled = true;
					appStackView.push(object);
				}

				appDB.getPage.connect(openTDayExercisesPage);
				appDB.openExercisesListPage();
			}
		} // bntAddExercise
	} //footer: ToolBar

	SimpleExercisesListPanel {
		id: exercisesPane
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

	function changeSplitLetter() {
		chkAdjustCalendar.visible = (cboSplitLetter.currentValue !== splitLetter);
		splitLetter = cboSplitLetter.currentValue;
		if (splitLetter === 'R')
			tDay = "0";
		else
		{
			if (tDay === "0")
				tDay = mesoCalendarModel.getLastTrainingDayBeforeDate(mainDate);
		}
		tDayModel.setSplitLetter(splitLetter);
		appDB.verifyTDayOptions(mainDate, splitLetter);
	}

	function gotExercise(strName1: string, strName2: string, nSets: string, nReps: string, nWeight: string) {
		function readyToProceed(object, id) {
			appDB.getItem.disconnect(readyToProceed);
			object.setAdded.connect(exerciseSetAdded);

			bStopBounce = true;
			if (navButtons === null)
				createNavButtons();
			scrollBarPosition = phantomItem.y;
			scrollTraining.scrollToPos(scrollBarPosition);
			bounceTimer.start();
			return;
		}

		appDB.getItem.connect(readyToProceed);
		itemManager.createExerciseObject(strName1 + " - " + strName2, nSets, nReps, nWeight);
	}

	function exerciseSetAdded(exercise_idx: int, scrollamount: int) {
		//bStopBounce = true;
		scrollTraining.setScrollBarPosition2(scrollamount/scrollTraining.contentHeight);
		//bounceTimer.start();
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
		itemManager.createSetObject(settype, tDayModel.setsNumber(exercise_idx), exerciseidx, "", "");
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		itemThatRequestedSimpleList = visible ? object : null;
		bEnableMultipleSelection = multipleSel;
		bShowSimpleExercisesList = visible;
	}

	function hideSimpleExerciseList() {
		exercisesPane.shown = false;
	}

	function showExercise(exerciseEntry: Item) {

	}

	function createFirstTimeTipComponent() {
		var component = Qt.createComponent("FirstTimeHomePageTip.qml", Qt.Asynchronous);
		function finishCreation() {
			firstTimeTip = component.createObject(trainingDayPage, { message:qsTr("Start here") });
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function placeTipOnAddExercise() {
		if (bFirstTime) {
			if (!firstTimeTip)
				createFirstTimeTipComponent();
			firstTimeTip.y = dayInfoToolBar.y;
			firstTimeTip.x = trainingDayPage.width-firstTimeTip.width;
			firstTimeTip.visible = true;
		}
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
			timerDialog.mins = mins;
			timerDialog.secs = secs;
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

		return;
		if (!bAlreadyLoaded) {
			if (bFirstTime) {
				if (grpIntent.visible) {
					scrollTraining.setScrollBarPosition(1);
					grpIntent.highlight = true;
				}
				else
					placeTipOnAddExercise();
			}
			bAlreadyLoaded = true;
		}
		else {
			if (navButtons)
				navButtons.visible = true;
		}
	}

	function pageDeActivation() {
		if (firstTimeTip)
			firstTimeTip.visible = false;
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
} // Page
