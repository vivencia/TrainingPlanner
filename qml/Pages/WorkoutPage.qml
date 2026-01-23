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
	imageSource: ":/images/backgrounds/backimage-workout.jpg"
	backgroundOpacity: 0.6
	objectName: "workoutPage"

	required property WorkoutManager workoutManager
	property DBExercisesModel workoutModel
	property WorkoutOrSplitExercisesList lstWorkoutExercises: null

	signal exerciseSelectedFromSimpleExercisesList();
	signal silenceTimeWarning();

	Connections {
		target: workoutModel
		function onExerciseCountChanged(): void {
			scrollTraining.setScrollBarPosition(workoutModel.exerciseCount === 1 ? 0 : 1);
			lstWorkoutExercises.positionViewAtIndex(workoutModel.workingExercise, ListView.Contain);
		}
	}

	Connections {
		target: workoutManager
		function onHaveNewWorkoutOptionsChanged(): void {
			if (workoutManager.haveNewWorkoutOptions())
				showIntentionDialog();
		}
	}

	onPageActivated: {
		if (!navButtons) {
			cboSplitLetter.currentIndex = Qt.binding(function() { return cboSplitLetter.indexOfValue(workoutModel.splitLetter); })
			createNavButtons();
			if (workoutManager.haveNewWorkoutOptions())
				showIntentionDialog();
		}
	}

	ScrollView {
		id: scrollTraining
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: layoutMain.height + exercisesFrame.height
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

		ScrollBar.vertical: ScrollBar {
			id: vBar
			policy: ScrollBar.AsNeeded

			onPositionChanged: {
				//const absoluteScrollY =  vBar.position * (scrollTraining.contentHeight - scrollTraining.height);
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
			margins: 5
		}

		TPButton {
			id: btnWorkoutInfo
			text: qsTr("--- WORKOUT INFO ---")
			imageSource: layoutMain.visible ? "fold-up.png" : "fold-down.png"
			imageSize: appSettings.itemSmallHeight * 0.8
			hasDropShadow: false
			flat: true
			rounded: false
			autoSize: true

			anchors {
				horizontalCenter: parent.horizontalCenter
				top: parent.top
			}

			onClicked: layoutMain.visible = !layoutMain.visible;
		}

		ColumnLayout {
			id: layoutMain
			width: parent.width
			spacing: 10

			anchors {
				top: btnWorkoutInfo.bottom
				topMargin: 10
				left: parent.left
				right: parent.right
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
					enabled: !workoutManager.workoutInProgress || workoutManager.workoutIsEditable
					width: appSettings.pageWidth * 0.2
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
					text: workoutManager.location()
					enabled: workoutManager.workoutIsEditable
					width: parent.width * 0.75

					onTextChanged: workoutManager.setLocation(text);
				}
			}

			Frame {
				id: frmTrainingTime
				visible: workoutModel.splitLetter !== "R"
				enabled: !workoutManager.workoutInProgress || workoutManager.workoutIsEditable
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

					TPButtonGroup {
						id: timeSessionGroup
					}

					TPButtonGroup {
						id: timeConstrainedSessionGroup
					}

					TPRadioButtonOrCheckBox {
						id: optTimeConstrainedSession
						text: qsTr("Time constrained session")
						buttonGroup: timeSessionGroup
						enabled: workoutManager.todaysWorkout && !workoutManager.workoutInProgress
						checked: false
						Layout.fillWidth: true
					}

					TPRadioButtonOrCheckBox {
						text: qsTr("By duration")
						buttonGroup: timeConstrainedSessionGroup
						visible: optTimeConstrainedSession.checked
						Layout.alignment: Qt.AlignCenter

						onClicked: showLimitedSessionTimerDialog();
					}

					TPRadioButtonOrCheckBox {
						text: qsTr("By time of day")
						buttonGroup: timeConstrainedSessionGroup
						visible: optTimeConstrainedSession.checked
						Layout.alignment: Qt.AlignCenter

						onClicked: restrictedTimeLoader.openDlg();
					}

					TPRadioButtonOrCheckBox {
						id: optFreeTimeSession
						text: qsTr("Open time training session")
						buttonGroup: timeSessionGroup
						checked: true
						enabled: workoutManager.todaysWorkout && !workoutManager.workoutInProgress
						Layout.fillWidth: true

						onClicked: workoutManager.prepareWorkOutTimer();
					}

					Loader {
						id: restrictedTimeLoader
						active: false
						asynchronous: true

						sourceComponent: TimePicker {
							id: dlgTimeEndSession
							hrsDisplay: appUtils.getHourFromCurrentTime()
							minutesDisplay: appUtils.getMinutesFromCurrentTime()
							bOnlyFutureTime: workoutManager.todaysWorkout ? workoutManager.editMode : false
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

					Row {
						enabled: workoutManager.todaysWorkout ? optFreeTimeSession.checked && workoutManager.editMode : true
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
							text: workoutManager.timeIn()
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

								onTimeSet: (hour, minutes) => workoutManager.setTimeIn(hour + ":" + minutes);
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
						enabled: optFreeTimeSession.checked && (workoutManager.editMode || workoutManager.todaysWorkout)
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
							text: workoutManager.timeOut()
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
								bOnlyFutureTime: workoutManager.todaysWorkout ? workoutManager.editMode : false

								onTimeSet: (hour, minutes) => workoutManager.setTimeOut(hour + ":" + minutes);
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
				text: workoutManager.notes()
				editable: workoutManager.workoutIsEditable
				visible: workoutModel.splitLetter !== "R"
				Layout.fillWidth: true

				onEditFinished: (new_text) => workoutManager.setNotes(new_text);
			}

			TPButton {
				text: qsTr("Use this workout exercises as the default exercises plan for the division ") + workoutModel.splitLetter + qsTr( " of this mesocycle")
				rounded: false
				visible: workoutManager.workoutFinished && workoutManager.haveExercises
				enabled: workoutManager.editMode;
				Layout.fillWidth: true
				Layout.minimumHeight: 3 * appSettings.itemDefaultHeight
				Layout.bottomMargin: 10

				onClicked: workoutManager.exportWorkoutToSplitPlan();
			}
		}// layoutMain

		TPButton {
			id: btnExercises
			text: qsTr("--- EXERCISES ---")
			imageSource: exercisesFrame.visible ? "fold-up.png" : "fold-down.png"
			imageSize: appSettings.itemSmallHeight * 0.8
			autoSize: true
			flat: true
			rounded: false
			hasDropShadow: false
			visible: workoutModel.splitLetter !== "R"

			anchors {
				horizontalCenter: parent.horizontalCenter
				top: layoutMain.visible ? layoutMain.bottom : btnWorkoutInfo.bottom
				topMargin: 20
			}

			onClicked: layoutMain.visible = !layoutMain.visible;
		}

		Item {
			id: exercisesFrame
			objectName: "exercisesFrame"
			height: appSettings.pageHeight
			visible: workoutModel.exerciseCount > 0

			anchors {
				top: btnExercises.bottom
				left: parent.left
				right: parent.right
			}
		}
	} // ScrollView scrollTraining

	footer: TPToolBar {
		id: dayInfoToolBar
		height: appSettings.pageHeight * 0.18
		visible: workoutModel.splitLetter !== "R"

		readonly property int buttonHeight: height * 0.45

		RowLayout {
			id: workoutLengthRow
			height: parent.height * 0.4
			spacing: 10

			anchors {
				left: parent.left
				leftMargin: 10
				top: parent.top
				topMargin: 0
				right: parent.right
				rightMargin: 0
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				width: parent.width * 0.2
				height: appSettings.itemDefaultHeight
				visible: workoutManager.todaysWorkout ? !workoutManager.workoutFinished && !workoutManager.editMode : false
				enabled: !workoutManager.workoutInProgress && workoutManager.todaysWorkout
				Layout.alignment: Qt.AlignLeft

				onClicked: workoutManager.startWorkout();
			}

			TPDigitalClock {
				id: hoursClock
				max: 24
				value: workoutManager.timerHour
				Layout.alignment: Qt.AlignCenter

			}
			Rectangle { color : appSettings.fontColor; width: 2; height: 35; Layout.alignment: Qt.AlignCenter }

			TPDigitalClock {
				id: minsClock
				max: 60
				value: workoutManager.timerMinute
				Layout.alignment: Qt.AlignCenter
			}
			Rectangle { color : appSettings.fontColor; width: 2; height: 35; Layout.alignment: Qt.AlignCenter }

			TPDigitalClock {
				id: secsClock
				max: 60
				value: workoutManager.timerSecond
				Layout.alignment: Qt.AlignCenter
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")
				width: parent.width * 0.2
				height: appSettings.itemDefaultHeight
				visible: btnStartWorkout.visible
				enabled: workoutManager.workoutInProgress

				onClicked: workoutManager.stopWorkout();
			}
		} //workoutLengthRow

		TPButton {
			id: btnFinishedDayOptions
			imageSource: "menu.png"
			rounded: false
			backgroundColor: appSettings.paneBackgroundColor
			width: height * 0.6
			height: dayInfoToolBar.buttonHeight
			visible: workoutManager.workoutFinished || !workoutManager.todaysWorkout || workoutManager.editMode
			Layout.alignment: Qt.AlignRight

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
			width: parent.width * 0.3
			height: dayInfoToolBar.buttonHeight
			visible: workoutManager.workoutFinished && workoutManager.haveExercises

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
			multiline: true
			rounded: false
			textUnderIcon: true
			visible: workoutModel.splitLetter !== "R"
			enabled: workoutManager.workoutIsEditable
			width: parent.width * 0.35
			height: dayInfoToolBar.buttonHeight

			anchors {
				right: parent.right
				rightMargin: 5
				top: workoutLengthRow.bottom
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: lstWorkoutExercises.appendNewExerciseToDivision();
		} // bntAddExercise
	} //footer: ToolBar

	property TPBalloonTip msgClearExercises: null
	function showClearExercisesMessage(): void {
		if (msgClearExercises === null) {
			function createMessageBox() {
				let component = Qt.createComponent("qrc:/qml/TPWidgets/TPBalloonTip.qml", Qt.Asynchronous);

				function finishCreation() {
					msgClearExercises = component.createObject(workoutPage, { parentPage: workoutPage, title: qsTr("Clear exercises list?"),
						keepAbove: true, message: qsTr("All exercises changes will be removed"), imageSource: "revert-day.png" } );
					msgClearExercises.button1Clicked.connect(function () { workoutManager.clearExercises(true);});
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

	property TPComplexDialog intentDlg: null
	function showIntentionDialog(): void {
		if (!intentDlg)
			createIntentionDialog();
		else {
			intentDlg.cboModel.clear();
			intentDlg.customStringProperty2 = workoutModel.splitLetter;
			intentDlg.customStringProperty3 = workoutManager.sessionLabel;
			intentDlg.customBoolProperty1 = workoutManager.canImportFromSplitPlan;
			intentDlg.customBoolProperty2 = workoutManager.canImportFromPreviousWorkout;
			intentDlg.customBoolProperty3 = !workoutManager.haveExercises;
			if (workoutManager.canImportFromPreviousWorkout) {
				const texts = workoutManager.previousWorkoutsList_text();
				const values = workoutManager.previousWorkoutsList_value();
				for (let i = 0; i < texts.length; ++i)
					intentDlg.cboModel.append({ text: texts[i], value: values[i], enabled: true });
			}
			intentDlg.show(-1);
		}
	}

	function createIntentionDialog(): void {
		let component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

		function finishCreation() {
			if (!intentDlg) {
				intentDlg = component.createObject(workoutPage, { parentPage: workoutPage, title: qsTr("What do you want to do today?"),
					button1Text: qsTr("Proceed"), button2Text: "", customItemSource:"TPTDayIntentGroup.qml" });
				intentDlg.button1Clicked.connect(intentChosen);
			}
			showIntentionDialog();
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function intentChosen(): void {
		switch (intentDlg.customIntProperty1) {
			case 1: //use meso plan
				workoutManager.getExercisesFromSplitPlan();
			break;
			case 2: //use previous day
				workoutManager.loadExercisesFromCalendarDay(intentDlg.customIntProperty2);
			break;
			case 3: //import from file
				workoutManager.importWorkout();
			break;
			case 4: //empty session
			break;
		}
	}

	//TODO remove maybe
	function placeExerciseIntoView(ypos: int): void {
		if (ypos === -1)
			ypos = 0;
		else if (ypos === -2)
			ypos = lstWorkoutExercises.y + scrollTraining.contentHeight;
		else
			ypos += lstWorkoutExercises.y;

		scrollTraining.contentItem.contentY = ypos;
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
	function showLimitedSessionTimerDialog(): void {
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
		let component = Qt.createComponent("qrc:/qml/ExercisesAndSets/PageScrollButtons.qml", Qt.Asynchronous);

		function finishCreation() {
			navButtons = component.createObject(workoutPage, { ownerPage: workoutPage });
			navButtons.scrollTo.connect(scrollTraining.setScrollBarPosition);
			navButtons.visible = Qt.binding(function() { return workoutModel.exerciseCount > 0; });
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	property TPFloatingMenuBar optionsMenu: null
	function showFinishedWorkoutOptions(): void {
		if (optionsMenu === null) {
			let optionsMenuMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			optionsMenu = optionsMenuMenuComponent.createObject(workoutPage, { titleHeader: qsTr("Non-current workout"),
																parentPage: workoutPage, width: appSettings.pageWidth * 0.6 });
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
