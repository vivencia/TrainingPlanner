pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import TpQml
import TpQml.Dialogs
import TpQml.Widgets
import TpQml.Exercises

import "./WorkoutElements"

TPPage {
	id: workoutPage
	imageSource: ":/images/backgrounds/backimage-workout.jpg"
	backgroundOpacity: 0.6
	objectName: "workoutPage"

//public:
	required property WorkoutManager workoutManager
	property DBExercisesModel workoutModel
	property WorkoutOrSplitExercisesList lstWorkoutExercises: null

	signal exerciseSelectedFromSimpleExercisesList();
	signal silenceTimeWarning();

//private:
	readonly property bool _can_export: workoutManager.workoutFinished && workoutManager.haveExercises

	Connections {
		target: workoutPage.workoutModel
		function onExerciseCountChanged(): void {
			scrollTraining.setScrollBarPosition(workoutPage.workoutModel.exerciseCount === 1 ? 0 : 1);
			workoutPage.lstWorkoutExercises.positionViewAtIndex(workoutPage.workoutModel.workingExercise, ListView.Contain);
		}
	}

	TPScrollView {
		id: scrollTraining
		parentPage: workoutPage
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
				text: workoutPage.workoutManager.headerText
				font: AppGlobals.largeFont
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
			}

			TPLabel {
				text: workoutPage.workoutManager.headerText_2
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
					enabled: !workoutPage.workoutManager.workoutInProgress || workoutPage.workoutManager.workoutIsEditable
					currentIndex: indexOfValue(workoutPage.workoutModel.splitLetter)
					Layout.preferredWidth: AppSettings.pageWidth * 0.2
					onActivated: (index) => workoutPage.workoutManager.changeSplitLetter(valueAt(index));
				} //TPComboBox
			}

			Row {
				visible: workoutPage.workoutModel.splitLetter !== "R"
				Layout.fillWidth: true
				spacing: 0
				padding: 0

				TPLabel {
					text: qsTr("Location:")
					width: parent.width * 0.25
				}
				TPTextInput {
					id: txtLocation
					text: workoutPage.workoutManager.location()
					enabled: workoutPage.workoutManager.workoutIsEditable
					width: parent.width * 0.75

					onTextChanged: workoutPage.workoutManager.setLocation(text);
				}
			}

			Frame {
				id: frmTrainingTime
				visible: workoutPage.workoutModel.splitLetter !== "R"
				enabled: !workoutPage.workoutManager.workoutInProgress || workoutPage.workoutManager.workoutIsEditable
				Layout.preferredHeight: AppSettings.pageHeight * 0.4
				Layout.fillWidth: true

				background: Rectangle {
					border.color: AppSettings.fontColor
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
						enabled: workoutPage.workoutManager.todaysWorkout && !workoutPage.workoutManager.workoutInProgress
						checked: false
						Layout.fillWidth: true
					}

					TPRadioButtonOrCheckBox {
						text: qsTr("By duration")
						buttonGroup: timeConstrainedSessionGroup
						visible: optTimeConstrainedSession.checked
						Layout.alignment: Qt.AlignCenter

						onClicked: workoutPage.showLimitedSessionTimerDialog();
					}

					TPRadioButtonOrCheckBox {
						text: qsTr("By time of day")
						buttonGroup: timeConstrainedSessionGroup
						visible: optTimeConstrainedSession.checked
						Layout.alignment: Qt.AlignCenter

						onClicked: restrictedTimeLoader.active = true;
					}

					TPRadioButtonOrCheckBox {
						id: optFreeTimeSession
						text: qsTr("Open time training session")
						buttonGroup: timeSessionGroup
						checked: true
						enabled: workoutPage.workoutManager.todaysWorkout && !workoutPage.workoutManager.workoutInProgress
						Layout.fillWidth: true

						onClicked: workoutPage.workoutManager.prepareWorkOutTimer();
					}

					Loader {
						id: restrictedTimeLoader
						asynchronous: true
						active: false

						property TimePicker _time_picker

						sourceComponent: TimePicker {
							id: dlgTimeEndSession
							hrsDisplay: AppUtils.getHourFromCurrentTime()
							minutesDisplay: AppUtils.getMinutesFromCurrentTime()
							bOnlyFutureTime: workoutPage.workoutManager.todaysWorkout ? workoutPage.workoutManager.editMode : false
							parentPage: workoutPage
							onTimeSet: (hour, minutes) => workoutPage.workoutManager.prepareWorkOutTimer(
																					AppUtils.getCurrentTimeString(), hour + ":" + minutes);
							onClosed: restrictedTimeLoader.active = false;
							Component.onCompleted: restrictedTimeLoader._time_picker = this;
						}

						onLoaded: _time_picker.showInWindow(-Qt.AlignCenter);
					}

					Row {
						enabled: optFreeTimeSession.checked && (workoutPage.workoutManager.editMode || workoutPage.workoutManager.todaysWorkout)
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
							text: workoutPage.workoutManager.timeIn
							horizontalAlignment: Text.AlignHCenter
							readOnly: true
							width: parent.width * 0.25
						}

						TPButton {
							id: btnInTime
							imageSource: "time.png"
							width: AppSettings.itemDefaultHeight
							height: width

							onClicked: timeInLoader.active = true;
						}

						Loader {
							id: timeInLoader
							asynchronous: true
							active: false

							property TimePicker _time_picker

							sourceComponent: TimePicker {
								id: dlgTimeIn
								hrsDisplay: AppUtils.getHourFromStrTime(txtInTime.text)
								minutesDisplay: AppUtils.getMinutesFromStrTime(txtInTime.text)
								parentPage: workoutPage
								onTimeSet: (hour, minutes) => workoutPage.workoutManager.timeIn = hour + ":" + minutes;
								onClosed: timeInLoader.active = false;
								Component.onCompleted: timeInLoader._time_picker = this;
							}

							onLoaded: _time_picker.show1(-1);
						}
					}

					Row {
						enabled: optFreeTimeSession.checked && (workoutPage.workoutManager.editMode || workoutPage.workoutManager.todaysWorkout)
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
							text: workoutPage.workoutManager.timeOut
							horizontalAlignment: Text.AlignHCenter
							readOnly: true
							width: parent.width * 0.25
						}

						TPButton {
							id: btnOutTime
							imageSource: "time.png"
							width: AppSettings.itemDefaultHeight
							height: width

							onClicked: timeOutLoader.active = true;
						}

						Loader {
							id: timeOutLoader
							asynchronous: true
							active: false

							property TimePicker _time_picker

							sourceComponent: TimePicker {
								id: dlgTimeOut
								hrsDisplay: AppUtils.getHourFromStrTime(txtOutTime.text)
								minutesDisplay: AppUtils.getMinutesFromStrTime(txtOutTime.text)
								parentPage: workoutPage
								bOnlyFutureTime: workoutPage.workoutManager.todaysWorkout ? workoutPage.workoutManager.editMode : false
								onTimeSet: (hour, minutes) => workoutPage.workoutManager.timeOut = hour + ":" + minutes;
								onClosed: timeOutLoader.active = false;
								Component.onCompleted: timeOutLoader._timer_picker = this;
							}

							onLoaded: _time_picker.showInWindow(-Qt.AlignCenter);
						}
					}
				} //ColumnLayout
			} //Frame

			SetNotesField {
				info: qsTr("This training session considerations:")
				text: workoutPage.workoutManager.notes()
				editable: workoutPage.workoutManager.workoutIsEditable
				visible: workoutPage.workoutModel.splitLetter !== "R"
				Layout.fillWidth: true

				onEditFinished: (new_text) => workoutPage.workoutManager.setNotes(new_text);
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
			visible: workoutPage.workoutModel.splitLetter !== "R"

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
			height: AppSettings.pageHeight
			visible: workoutPage.workoutModel.exerciseCount > 0

			anchors {
				top: btnExercises.bottom
				left: parent.left
				right: parent.right
			}
		}
	} // ScrollView scrollTraining

	footer: TPToolBar {
		id: dayInfoToolBar
		height: workoutPage.footerHeight
		visible: workoutPage.workoutModel.splitLetter !== "R"

		readonly property int buttonHeight: AppSettings.itemDefaultHeight * 2.3

		RowLayout {
			id: workoutLengthRow
			height: dayInfoToolBar.height * 0.4
			visible: workoutPage.workoutManager.todaysWorkout
			spacing: 10

			anchors {
				left: dayInfoToolBar.left
				leftMargin: 10
				top: dayInfoToolBar.top
				topMargin: 0
				right: dayInfoToolBar.right
				rightMargin: 0
			}

			TPButton {
				id: btnStartWorkout
				text: qsTr("Begin")
				enabled: !workoutPage.workoutManager.workoutInProgress
				Layout.preferredWidth: dayInfoToolBar.width * 0.2
				Layout.preferredHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignLeft

				onClicked: workoutPage.workoutManager.startWorkout();
			}

			TPDigitalClock {
				id: hoursClock
				max: 24
				value: workoutPage.workoutManager.timerHour
				Layout.alignment: Qt.AlignCenter

			}

			Rectangle {
				color : AppSettings.fontColor
				Layout.preferredWidth: 2
				Layout.preferredHeight: 35
				Layout.alignment: Qt.AlignCenter
			}

			TPDigitalClock {
				id: minsClock
				max: 60
				value: workoutPage.workoutManager.timerMinute
				Layout.alignment: Qt.AlignCenter
			}

			Rectangle {
				color : AppSettings.fontColor
				Layout.preferredWidth: 2
				Layout.preferredHeight: 35
				Layout.alignment: Qt.AlignCenter
			}

			TPDigitalClock {
				id: secsClock
				max: 60
				value: workoutPage.workoutManager.timerSecond
				Layout.alignment: Qt.AlignCenter
			}

			TPButton {
				id: btnEndWorkout
				text: qsTr("Finish")
				enabled: workoutPage.workoutManager.workoutInProgress
				Layout.preferredWidth: dayInfoToolBar.width * 0.2
				Layout.preferredHeight: AppSettings.itemDefaultHeight

				onClicked: workoutPage.workoutManager.stopWorkout();
			}
		} //workoutLengthRow

		TPButton {
			id: btnOptions
			imageSource: "menu.png"
			rounded: false
			backgroundColor: AppSettings.paneBackgroundColor
			width: height * 0.6
			height: dayInfoToolBar.buttonHeight
			Layout.alignment: Qt.AlignRight

			anchors {
				left: parent.left
				leftMargin: 5
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: workoutPage.showWorkoutOptionsMenu();
		}

		TPButton {
			text: qsTr("Add exercise")
			imageSource: "exercises-add.png"
			multiline: true
			rounded: false
			textUnderIcon: true
			visible: workoutPage.workoutModel.splitLetter !== "R"
			enabled: workoutPage.workoutManager.workoutIsEditable
			width: parent.width * 0.35
			height: dayInfoToolBar.buttonHeight

			anchors {
				right: parent.right
				rightMargin: 5
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: {
				//Force triggering of onEditFinished of the last text control to receive input
				workoutPage.lstWorkoutExercises.forceActiveFocus();
				workoutPage.lstWorkoutExercises.appendNewExerciseToDivision();
			}
		}
	} //footer: ToolBar

	Loader {
		id: newWorkoutOptionsLoader
		asynchronous: true
		active: workoutPage.workoutManager.haveNewWorkoutOptions

		property WorkoutOptionsDialog _options_dlg
		property ListModel _prev_workouts_list

		sourceComponent: WorkoutOptionsDialog {
			workoutModel: workoutPage.workoutModel
			workoutManager: workoutPage.workoutManager
			parentWorkoutPage: workoutPage
			prevWorkoutsList: newWorkoutOptionsLoader._prev_workouts_list

			onSelectedOption: (option) => {
				switch (option) {
				case 0: workoutPage.workoutManager.getExercisesFromSplitPlan(); break;
				case 1: workoutPage.workoutManager.loadExercisesFromCalendarDay(this.selectedPrevWorkoutDay); break;
				case 2: workoutPage.workoutManager.importWorkout(); break;
				case 3: break; //leave it empty
				}
			}
			onClosed: newWorkoutOptionsLoader.active = false;
			Component.onCompleted: newWorkoutOptionsLoader._options_dlg = this;
		}

		onLoaded: {
			if (workoutPage.workoutManager.canImportFromPreviousWorkout) {
				_prev_workouts_list.clear();
				const texts = workoutPage.workoutManager.previousWorkoutsList_text();
				const values = workoutPage.workoutManager.previousWorkoutsList_value();
				for (let i = 0; i < texts.length; ++i)
					_prev_workouts_list.append({ "text": texts[i], "value": values[i], "icon": "", "enabled": true });
			}
			_options_dlg.showInWindow(-Qt.AlignCenter);
		}
	}

	Loader {
		id: sessionLengthLoader
		asynchronous: true
		active: false

		property TimerDialog _timer_dlg

		sourceComponent: TimerDialog {
			parentPage: workoutPage
			timePickerOnly: true
			windowTitle: qsTr("Length of this training session")
			onClosed: sessionLengthLoader.active = false;
			onUseTime: (strtime) => workoutPage.workoutManager.prepareWorkOutTimer(strtime);
			Component.onCompleted: sessionLengthLoader._timer_dlg = this;
		}

		onLoaded: _timer_dlg.show1(-1);
	}

	property TimerDialog dlgSessionLength: null
	function showLimitedSessionTimerDialog(): void {
		sessionLengthLoader.active = true;
	}

	Loader {
		id: timeWarningDialogLoader
		asynchronous: true
		active: false

		property string timeleft
		property bool bmin
		property TPBalloonTip _warning_dlg
		readonly property int timeout: bmin ? 60000 : 15000

		sourceComponent: TPBalloonTip {
			parentPage: workoutPage
			title: qsTr("Attention!")
			message: "<b>" + timeWarningDialogLoader.timeleft + (timeWarningDialogLoader.bmin ? qsTr(" minutes") :
																			qsTr(" seconds")) + qsTr("</b> until end of training session!");
			button1Text: qsTr("I'm almost finished!")
			button2Text: ""
			imageSource: "sound-off"
			onClosed: {
				workoutPage.silenceTimeWarning();
				timeWarningDialogLoader.active = false;
			}
			Component.onCompleted: timeWarningDialogLoader._waring_dlg = this;
		}

		onLoaded: _warning_dlg.showTimed(timeout, Qt.AlignTop|Qt.AlignHCenter);
	}
	function displayTimeWarning(timeleft: string, bmin: bool): void {
		timeWarningDialogLoader.timeleft = timeleft;
		timeWarningDialogLoader.btin = bmin;
		timeWarningDialogLoader.active = true;
	}

	Loader {
		id: workoutOptionsLoader
		asynchronous: true
		active: false

		property TPFloatingMenuBar _menu_bar

		sourceComponent: TPFloatingMenuBar {
			parentPage: workoutPage
			entriesList: [
				{ "label": qsTr("Import"), "image": "import.png", "id": 0, "visible": true },
				{ "label": qsTr("Export"), "image": "save-day.png", "id": 1, "visible": workoutPage._can_export },
				{ "label": qsTr("Share"), "image": "export.png", "id": 2, "visible": workoutPage._can_export && Qt.platform.os === "android"},
				{ "label": qsTr("Use this workout exercises as the default exercises plan for the division ") +
					workoutPage.workoutModel.splitLetter + qsTr( " of this mesocycle"), "id": 3, "visible": workoutPage._can_export },
				{ "label": qsTr("Edit workout"), "image": "edit.png", "id": 4, "visible": workoutPage.workoutManager.workoutFinished && workoutPage.workoutManager.todaysWorkout },
				{ "label": qsTr("Reset Workout"), "image": "reset.png", "id": 5, "visible": workoutPage.workoutManager.todaysWorkout },
			]

			onMenuEntrySelected: (id) => {
				switch (id) {
				case 0:
					workoutPage.workoutManager.importWorkout();
					break;
				case 1:
					workoutPage.showExportDlg(false);
					break;
				case 2:
					workoutPage.showExportDlg(true);
					break;
				case 3: workoutPage.workoutManager.exportWorkoutToSplitPlan(); break;
				case 4:
					workoutPage.workoutManager.editMode = !workoutPage.workoutManager.editMode;
					workoutOptionsLoader._menu_bar.entriesList.set(4).label = workoutPage.workoutManager.editMode ?
																								qsTr("Editing done") : qsTr("Edit workout");
					break;
				case 5:
					workoutPage.showResetWorkoutDialog();
					break;
				}
			}

			onClosed: workoutOptionsLoader.active = false;
			Component.onCompleted: workoutOptionsLoader._menu_bar = this;
		}

		onLoaded: _menu_bar.showByWidget(btnOptions, Qt.AlignTop);
	}
	function showWorkoutOptionsMenu(): void {
		workoutOptionsLoader.active = true;
	}

	Loader {
		id: exportDlgLoader
		active: false
		asynchronous: true

		property bool share
		property TPBalloonTip _export_dlg

		sourceComponent: TPBalloonTip {
			id: exportTypeTip
			title: exportDlgLoader.share ? qsTr("Share workout?") : qsTr("Export workout to file?")
			imageSource: "export.png"
			parentPage: workoutPage
			closeButtonVisible: true

			onButton1Clicked: workoutPage.workoutManager.exportWorkout(exportDlgLoader.share);
			onClosed: exportDlgLoader.active = false;
			Component.onCompleted: exportDlgLoader._export_dlg = this;
		}

		onLoaded: {
			if (status === Loader.Ready)
				_export_dlg.showInWindow(-Qt.AlignCenter);
		}
	}
	function showExportDlg(share: bool): void {
		exportDlgLoader.share = share;
		exportDlgLoader.active = true;
	}

	Loader {
		id: resetWorkoutDlgLoader
		asynchronous: true
		active: false

		property TPBalloonTip _reset_dlg
		sourceComponent: TPBalloonTip {
			parentPage: workoutPage
			title: qsTr("Reset workout?")
			message: qsTr("Exercises will not be afected")
			imageSource: "reset.png"
			onClosed: resetWorkoutDlgLoader.active = false;
			onButton1Clicked: workoutPage.workoutManager.resetWorkout();
			Component.onCompleted: resetWorkoutDlgLoader._reset_dlg = this;
		}

		onLoaded: _reset_dlg.show(-1);
	}
	function showResetWorkoutDialog(): void {
		resetWorkoutDlgLoader.active = true;
	}

	function getExerciseNameFieldYPos(): int {
		return lstWorkoutExercises.exerciseNameFieldYPosition();
	}

} // Page
