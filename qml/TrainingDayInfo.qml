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

	required property date mainDate //dayDate
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

	property date sessionStart

	property bool bFirstTime: false
	property bool bAlreadyLoaded
	property bool bStopBounce: false

	property bool bDayIsFinished: false
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

	onBDayIsFinishedChanged : {
		if (bDayIsFinished)
			sessionLength = runCmd.calculateTimeBetweenTimes(timeIn, timeOut);
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
			timeOut = runCmd.addToTime(timeIn, 1, 30);
			tDayModel.setTimeIn(timeIn);
			bDayIsFinished = false;
		}
	}

	TimePicker {
		id: dlgTimeOut
		hrsDisplay: runCmd.getHourOrMinutesFromStrTime(txtOutTime.text)
		minutesDisplay: runCmd.getMinutesOrSeconsFromStrTime(txtOutTime.text)

		onTimeSet: (hour, minutes) => {
			timeOut = hour + ":" + minutes;
			tDayModel.setTimeOut(timeOut);
			if (tDayModel.exerciseCount > 0)
			{
				bDayIsFinished = true;
				if (btnFloat)
					btnFloat.visible = false;
				btnSaveDay.clicked();
			}
		}
	}

	TimerDialog {
		id: dlgSessionLength
		timePickerOnly: true
		windowTitle: qsTr("Length of this training session")

		onUseTime: (strtime) => workoutTimer.prepare(strtime);
	}

	TimePicker {
		id: dlgTimeEndSession
		hrsDisplay: runCmd.getCurrentTimeString()
		minutesDisplay: runCmd.getCurrentTimeString()

		onTimeSet: (hour, minutes) => workoutTimer.prepare(hour + ":" + minutes);
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
		property int nShow: 0

		onOpened: nShow++;
		onButton1Clicked: playSound.stop();
	}

	TPBalloonTip {
		id: timerDlgMessage
		title: qsTr("Attention!")
		message: qsTr("Only one timer window can be opened at a time!")
		imageSource: "qrc:/images/"+darkIconFolder+"time.png"
		button1Text: qsTr("OK")
		highlightMessage: true
	}

	Timer {
		id: workoutTimer
		interval: 1000
		running: false
		repeat: true

		property int hours: 0
		property int mins: 0
		property int secs: 0
		property bool forward: true
		property bool bWasSuspended: false

		onTriggered: calcTime();

		Component.onCompleted: {
			if (Qt.platform.os === "android") {
				mainwindow.appSuspended.connect(appSuspended);
				mainwindow.appActive.connect(appResumed);
			}
		}

		function appSuspended() {
			if (running)
				bWasSuspended = true;
		}

		function appResumed() {
			if (bWasSuspended) {
				const elapsedTime = runCmd.calculateTimeBetweenTimes(sessionStart, runCmd.getCurrentTimeString(true));
				console.log("############## elapsedTime", elapsedTime.toTimeString());

				var correctedCounter;
				if (forward)
					correctedCounter = runCmd.calculateTimeBetweenTimes("00:00::00", elapsedTime);
				else
					correctedCounter = runCmd.calculateTimeBetweenTimes(elapsedTime, runCmd.formatTime(sessionLength, true));
				hours = correctedCounter.getHours();
				mins = correctedCounter.getMinutes();
				secs = correctedCounter.getSeconds();
				bWasSuspended = false;
			}
		}

		function prepare(strTime: string) {
			secs = 0;
			mins = runCmd.getMinutesOrSeconsFromStrTime(strTime)*1
			hours = runCmd.getHourOrMinutesFromStrTime(strTime)*1;
		}

		function calcTime() {
			if (bWasSuspended) return;
			if (forward) {
				if (secs == 59) {
					secs = 0;
					if (mins === 59) {
						mins = 0;
						hours++;
					}
					mins++;
				}
				secs++;
			}
			else {
				if (secs == 0) {
					if (mins === 0) {
						if (hours === 0) {
							forward = true;
							return;
						}
						mins = 59;
						hours--;
					}
					else {
						if (hours === 0) {
							switch (mins) {
								case 15:
									if (tipTimeWarn.nShow === 0) {
										tipTimeWarn.timeLeft = String(mins) + qsTr("minute(s)");
										playSound.play();
										tipTimeWarn.showTimed(18000, 0);
									}
								break;
								case 5:
									if (tipTimeWarn.nShow === 0)
										tipTimeWarn.nShow = 1;
									if (tipTimeWarn.nShow === 1) {
										tipTimeWarn.timeLeft = String(mins) + qsTr("minute(s)");
										playSound.play();
										tipTimeWarn.showTimed(18000, 0);
									}
								break;
								case 0:
									if (tipTimeWarn.nShow !== 2)
										tipTimeWarn.nShow = 2;
									if (tipTimeWarn.nShow === 2) {
										tipTimeWarn.timeLeft = String(secs) + qsTr("second(s)");
										playSound.loops = 4;
										playSound.play();
										tipTimeWarn.showTimed(60000, 0);
									}
								break;
							}
						}
					}
					secs = 59;
					mins--;
				}
				secs--;
			}
		}
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
						enabled: !workoutTimer.running
						Layout.fillWidth: true

						onClicked: workoutTimer.forward = true;
					}
					TPRadioButton {
						id: optTimeConstrainedSession
						text: qsTr("Time constrained session")
						checked: false
						Layout.fillWidth: true

						onClicked: workoutTimer.forward = false;
					}

					RowLayout {
						Layout.fillWidth: true
						Layout.leftMargin: 30
						Layout.bottomMargin: 10

						TPButton {
							id: btnTimeLength
							text: qsTr("By duration")
							enabled: !workoutTimer.running
							visible: optTimeConstrainedSession.checked
							Layout.alignment: Qt.AlignCenter
							onClicked: dlgSessionLength.open();
						}
						TPButton {
							id: btnTimeHour
							text: qsTr("By time of day")
							enabled: !workoutTimer.running
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

							Component.onCompleted: {
								timeIn = tDayModel.timeIn();
								if (timeIn.length === 0) {
									timeIn = runCmd.getCurrentTimeString();
									tDayModel.setTimeIn(timeIn);
								}
							}
						}

						RoundButton {
							id: btnInTime
							width: 40
							height: 40
							enabled: !workoutTimer.running
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

							Component.onCompleted: {
								timeOut = tDayModel.timeOut();
								if (timeOut.length === 0) {
									timeOut = runCmd.formatFutureTime(1, 30);
									tDayModel.setTimeOut(timeOut);
								}
							}
						}

						RoundButton {
							id: btnOutTime
							enabled: !workoutTimer.running
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
				visible: bDayIsFinished

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
		if (Qt.platform.os === "android")
			mainwindow.appSuspended.connect(aboutToBeSuspended);
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
			color: AppSettings.primaryDarkColor
			opacity: 0.7
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
				text: qsTr("Workout:")
				color: AppSettings.fontColor
				font.bold: true
				font.pointSize: AppSettings.fontSizeText
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")

				onClicked: {
					workoutTimer.start();
					timeIn = runCmd.getCurrentTimeString();
					tDayModel.setTimeIn(timeIn);
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

					DigitalClock { max: 24; value: workoutTimer.hours; }
					Rectangle { color : AppSettings.fontColor; width: 2; height: 35 }
					DigitalClock { max: 60; value: workoutTimer.mins; }
					Rectangle { color : AppSettings.fontColor; width: 2; height: 35 }
					DigitalClock { max: 60; value: workoutTimer.secs; }
				}
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")

				onClicked: {
					workoutTimer.stop();
					timeOut = runCmd.formatFutureTime(1, 30);
					tDayModel.setTimeOut(timeOut);
					tDayModel.modified = true;
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

	function exerciseSetAdded(exerciseObjIdx, setObject) {
		bStopBounce = true;
		if (exerciseObjIdx === tDayModel.exerciseCount - 1)
			scrollBarPosition = phantomItem.y;
		else
			scrollBarPosition = phantomItem.y - lblExercisesStart.y + setObject.y + setObject.height;
		scrollTraining.scrollToPos(scrollBarPosition);
		bounceTimer.start();
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

	function aboutToBeSuspended() {
		if (tDayModel.modified)
			btnSaveDay.clicked();
	}
} // Page
