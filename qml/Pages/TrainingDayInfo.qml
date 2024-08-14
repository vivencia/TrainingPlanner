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

TPPage {
	id: trainingDayPage
	objectName: "trainingDayPage"

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
	property bool dayIsNotCurrent: false
	property bool bCalendarChangedPending: false

	property date previousDivisionDayDate
	property TPComplexDialog intentDlg: null
	property TPFloatingButton btnFloat: null
	property PageScrollButtons navButtons: null
	property TimerDialog timerDialog: null
	property var timerDialogRequester: null

	property bool intentDialogShown: splitLetter !== "R" && (bHasMesoPlan || bHasPreviousTDays || tDayModel.exerciseCount === 0)

	onPageActivated: {
		itemManager.setCurrenttDay(mainDate);
		changeComboModel();
	}

	onPageOptionsLoadedChanged: {
		if (pageOptionsLoaded && intentDialogShown) {
			showIntentionDialog();
			pageOptionsLoaded = false;
		}
	}

	signal mesoCalendarChanged()

	property var splitModel: [ { value:'A', text:'A' }, { value:'B', text:'B' }, { value:'C', text:'C' },
							{ value:'D', text:'D' }, { value:'E', text:'E' }, { value:'F', text:'F' }, { value:'R', text:'R' } ]

	property TPFloatingMenuBar imexportMenu: null
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

	ListModel {
		id: cboModel
	}

	TimePicker {
		id: dlgTimeIn
		hrsDisplay: runCmd.getHourOrMinutesFromStrTime(txtInTime.text)
		minutesDisplay: runCmd.getMinutesOrSeconsFromStrTime(txtInTime.text)
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
		hrsDisplay: runCmd.getHourOrMinutesFromStrTime(txtOutTime.text)
		minutesDisplay: runCmd.getMinutesOrSeconsFromStrTime(txtOutTime.text)
		parentPage: trainingDayPage

		Component.onCompleted: {
			if (runCmd.areDatesTheSame(mainDate, new Date()))
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
			const workoutLenght = runCmd.calculateTimeDifference(timeIn, timeOut);
			updateTimer(workoutLenght.getHours(), workoutLenght.getMinutes(), workoutLenght.getSeconds());
			appDB.setDayIsFinished(mainDate, true);
			itemManager.rollUpExercises();
		}
		else {
			if (runCmd.areDatesTheSame(mainDate, new Date())) {
				optTimeConstrainedSession.checked = true;
				workoutTimer.stopWatch = false;
				workoutTimer.prepareTimer(runCmd.calculateTimeDifference_str(runCmd.getCurrentTimeString(), timeOut));
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
		hrsDisplay: runCmd.getHourFromCurrentTime()
		minutesDisplay: runCmd.getMinutesFromCurrentTime()
		bOnlyFutureTime: true
		parentPage: trainingDayPage

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

	property TPBalloonTip msgRemoveExercise: null
	function showRemoveExerciseMessage(exerciseidx: int) {
		if (!AppSettings.alwaysAskConfirmation) {
			itemManager.removeExerciseObject(exerciseidx);
			return;
		}

		if (msgRemoveExercise === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveExercise = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Remove Exercise?"),
						button1Text: qsTr("Yes"), button2Text: qsTr("No"), imageSource: "remove.png" } );
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
		if (!AppSettings.alwaysAskConfirmation) {
			itemManager.removeSetObject(setnumber, exerciseidx);
			return;
		}

		if (msgRemoveSet === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgRemoveSet = component.createObject(trainingDayPage, { parentPage: trainingDayPage, imageSource: "remove.png",
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
					msgClearExercises.button1Clicked.connect(function () { appDB.clearExercises(); } );
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

	property TPComplexDialog exportMessage: null
	function showExportMessage(share: bool) {
		if (exportMessage === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					exportMessage = component.createObject(trainingDayPage, { parentPage: trainingDayPage, customStringProperty1: lblHeader.text,
						customStringProperty2: qsTr("Human readable?"), customStringProperty3: "export.png", button1Text: qsTr("Yes"), button2Text: qsTr("No"),
						customItemSource: "TPDialogWithMessageAndCheckBox.qml" } );
					exportMessage.button1Clicked.connect(function () { appDB.exportTrainingDay(mainDate, splitLetter, bShare, checkBoxChecked); } );
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		exportMessage.title = share ? qsTr("Share workout?") : qsTr("Export workout to file?");
		exportMessage.show(-1);
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

	property TPBalloonTip calChangedWarningMessage: null
	function warnCalendarChanged(newsplitletter: string, newtday: string, newsplittext: string) {

		function acceptChanges() {
			bCalendarChangedPending = false;
			splitLetter = newsplitletter;
			tDay = newtday;
			splitText = newsplittext;
			tDayModel.setTrainingDay(tDay);
			tDayModel.setSplitLetter(splitLetter);
			showClearExercisesMessage();
			saveWorkout();
		}

		if (calChangedWarningMessage === null) {
			function createMessageBox() {
				var component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					calChangedWarningMessage = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title: qsTr("Calendar changed! Update?"),
						message: qsTr("Exercises will not be afected"), button1Text: qsTr("Yes"), button2Text: qsTr("No"), imageSource: "warning.png" } );
					tipTimeWarn.button1Clicked.connect(acceptChanges);
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createMessageBox();
		}
		calChangedWarningMessage.message = qsTr("Training division: ") + splitLetter + " -> " + newsplitletter + qsTr("\nWorkout number: ") + tDay + " -> " + newtday
		bCalendarChangedPending = true;
		calChangedWarningMessage.show(-1);
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
					enabled: tDayModel.dayIsEditable
					Layout.row: 1
					Layout.column: 1
					Layout.fillWidth: true

					onTextChanged: tDayModel.setLocation(text);
				}
			}

			Frame {
				id: frmTrainingTime
				visible: splitLetter !== 'R' && !intentDialogShown
				enabled: workoutTimer.active ? false : !editMode && !tDayModel.dayIsFinished
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
							enabled: editMode || !runCmd.areDatesTheSame(mainDate, new Date())

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
				readOnly: tDayModel.dayIsEditable//!tDayModel.dayIsFinished
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
					enabled: tDayModel.dayIsEditable//!tDayModel.dayIsFinished
					visible: exercisesLayout.children.length > 0
					ToolTip.text: "Remove all exercises"
					imageName: "revert-day.png"

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

	Component.onDestruction: {
		if (bCalendarChangedPending)
			calChangedWarningMessage.acceptChanges();
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
		visible: splitLetter !== 'R'

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
				text: !tDayModel.dayIsFinished ? qsTr("Workout:") : qsTr("Workout session length: ")
				color: AppSettings.fontColor
				font.bold: true
				font.pointSize: AppSettings.fontSizeText
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				flat: false
				visible: dayIsNotCurrent ? false : !tDayModel.dayIsFinished && !editMode && !intentDialogShown
				enabled: !workoutTimer.active

				onClicked: {
					if (timeIn.indexOf("-") === -1)
						workoutTimer.prepareTimer(runCmd.getCurrentTimeString());
					timeIn = runCmd.getCurrentTimeString();
					workoutTimer.startTimer(timeIn);
					tDayModel.setTimeIn(timeIn);
					tDayModel.dayIsEditable = true;
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
				visible: btnStartWorkout.visible
				enabled: workoutTimer.active

				onClicked: {
					workoutTimer.stopTimer();
					const sessionLength = workoutTimer.elapsedTime();
					updateTimer(sessionLength.getHours(), sessionLength.getMinutes(), sessionLength.getSeconds());
					timeOut = runCmd.getCurrentTimeString();
					tDayModel.setTimeOut(timeOut);
					tDayModel.dayIsEditable = false;
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
			visible: tDayModel.dayIsFinished || dayIsNotCurrent

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
			//enabled: !tDayModel.dayIsFinished ? editMode ? splitLetter !== 'R' : splitLetter !== 'R' && workoutTimer.active : false;
			enabled: tDayModel.dayIsEditable

			anchors {
				right: parent.right
				rightMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			property bool bFirstClick: true

			onClicked: {
				appDB.openExercisesListPage(true, bFirstClick ? trainingDayPage : null);
				bFirstClick = false;
			}
		} // bntAddExercise
	} //footer: ToolBar

	SimpleExercisesListPanel {
		id: exercisesPane
		parentPage: trainingDayPage
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
		if (!intentDlg) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				intentDlg = component.createObject(trainingDayPage, { parentPage: trainingDayPage, title:qsTr("What do you want to do today?"),
					button1Text: qsTr("Proceed"), customItemSource:"TPTDayIntentGroup.qml", bClosable: false, customBoolProperty1: bHasMesoPlan,
					customModel: previousTDays,	customBoolProperty2: bHasPreviousTDays, customBoolProperty3: tDayModel.exerciseCount === 0 });
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
				appDB.loadExercisesFromMesoPlan(splitLetter);
			break;
			case 2: //use previous day
				appDB.loadExercisesFromDate(intentDlg.customStringProperty1);
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
			optionsMenu.addEntry(qsTr("Reset Workout"), "reset.png", 1, true);
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
					itemManager.rollUpExercises();
					appDB.setDayIsFinished(mainDate, true);
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
