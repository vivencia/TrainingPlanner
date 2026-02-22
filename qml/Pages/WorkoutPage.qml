import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "../"
import "../Dialogs"
import "../ExercisesAndSets"
import "../TPWidgets"
import "WorkoutElements"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: workoutPage
	imageSource: ":/images/backgrounds/backimage-workout.jpg"
	backgroundOpacity: 0.6
	objectName: "workoutPage"

	required property WorkoutManager workoutManager
	property DBExercisesModel workoutModel
	property WorkoutOrSplitExercisesList lstWorkoutExercises: null
	readonly property int bottomBarHeight: footerHeight + (workoutManager.todaysWorkout ? appSettings.itemExtraLargeHeight : 0)

	signal exerciseSelectedFromSimpleExercisesList();
	signal silenceTimeWarning();

	Connections {
		target: workoutModel
		function onExerciseCountChanged(): void {
			scrollTraining.setScrollBarPosition(workoutModel.exerciseCount === 1 ? 0 : 1);
			lstWorkoutExercises.positionViewAtIndex(workoutModel.workingExercise, ListView.Contain);
		}
	}

	onPageActivated: {
		if (!navButtons) {
			cboSplitLetter.currentIndex = Qt.binding(function() { return cboSplitLetter.indexOfValue(workoutModel.splitLetter); })
			createNavButtons();
		}
	}

	TPScrollView {
		id: scrollTraining
		contentHeight: layoutMain.height + exercisesFrame.height

		anchors {
			fill: parent
			margins: 5
		}

		TPButton {
			id: btnWorkoutInfo
			text: qsTr("--- WORKOUT INFO ---")
			imageSource: layoutMain.visible ? "fold-up.png" : "fold-down.png"
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
						enabled: optFreeTimeSession.checked && (workoutManager.editMode || workoutManager.todaysWorkout)
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
								bOnlyFutureTime: workoutManager.todaysWorkout ? workoutManager.editMode : false

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

			onClicked: exercisesFrame.visible = !exercisesFrame.visible;
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
		height: bottomBarHeight
		visible: workoutModel.splitLetter !== "R"

		readonly property int buttonHeight: appSettings.itemDefaultHeight * 2.3

		RowLayout {
			id: workoutLengthRow
			height: parent.height * 0.4
			visible: workoutManager.todaysWorkout
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
				enabled: !workoutManager.workoutInProgress
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
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: {
				lstWorkoutExercises.forceActiveFocus();//Force triggering of onEditFinished of the last text control to receive input
				lstWorkoutExercises.appendNewExerciseToDivision();
			}
		}
	} //footer: ToolBar

	Loader {
		active: workoutManager.haveNewWorkoutOptions
		asynchronous: true
		sourceComponent: WorkoutOptionsDialog {
			workoutModel: workoutPage.workoutModel
			workoutManager: workoutPage.workoutManager
			parentWorkoutPage: workoutPage

			property ListModel prevWorkoutsList

			onSelectedOption: (option) => {
				switch (option) {
					case 0: workoutManager.getExercisesFromSplitPlan(); break;
					case 1: workoutManager.loadExercisesFromCalendarDay(intentDlg.customIntProperty2); break;
					case 2: workoutManager.importWorkout(); break;
					case 3: break; //leave it empty
				}
			}
		}

		onLoaded: {
			if (workoutManager.canImportFromPreviousWorkout) {
				prevWorkoutsList.clear();
				const texts = workoutManager.previousWorkoutsList_text();
				const values = workoutManager.previousWorkoutsList_value();
				for (let i = 0; i < texts.length; ++i)
					prevWorkoutsList.append({ text: texts[i], value: values[i], enabled: true });
			}
			item.show1(-1);
		}
	}

	//TODO remove maybe
	/*function placeExerciseIntoView(ypos: int): void {
		if (ypos === -1)
			ypos = 0;
		else if (ypos === -2)
			ypos = lstWorkoutExercises.y + scrollTraining.contentHeight;
		else
			ypos += lstWorkoutExercises.y;

		scrollTraining.contentItem.contentY = ypos;
	}*/

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

	function getExerciseNameFieldYPos(): int {
		return lstWorkoutExercises.exerciseNameFieldYPosition();
	}
} // Page
