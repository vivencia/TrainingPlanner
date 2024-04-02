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

	property date sessionLength

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
		color: primaryDarkColor
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
			if (tDayModel.exercisesNumber() > 0)
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

		onUseTime: (strtime) => {
			sessionLength = runCmd.timeFromStrTime(strtime);
			timeOut = runCmd.formatFutureTime(sessionLength);
			tDayModel.setTimeOut(timeOut);
			timerRestricted.init(timeOut);
		}
	}

	TimePicker {
		id: dlgTimeEndSession
		hrsDisplay: runCmd.getCurrentTimeString()
		minutesDisplay: runCmd.getCurrentTimeString()

		onTimeSet: (hour, minutes) => {
			timeOut = hour + ":" + minutes;
			tDayModel.setTimeOut(timeOut);
			timerRestricted.init(timeOut);
		}
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
		message: "<b>" + displayMin + qsTr("</b> minute(s) until end of training session!")
		imageSource: "qrc:/images/"+darkIconFolder+"sound-off.png"
		button1Text: qsTr("OK")

		property string displayMin
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
		id: timerRestricted
		interval: 60000 //Every one minute
		repeat: true
		property bool complete: false
		property string finalHour
		property string finalMin

		onTriggered: {
			sessionLength = runCmd.calculateTimeRemaing(timeOut);
			const hour = runCmd.getHourFromCurrentTime();
			if (hour === finalHour) {
				const min = runCmd.getMinutesFromCurrentTime();
				const timeLeft = parseInt(finalMin) - parseInt(min);
				if (timeLeft <= 15) {
					switch (tipTimeWarn.nShow) {
						case 0:
							tipTimeWarn.displayMin = timeLeft.toString();
							playSound.play();
							tipTimeWarn.showTimed(18000, 0);
						break;
						case 1:
							if (timeLeft <= 5) {
								tipTimeWarn.displayMin = timeLeft.toString();
								playSound.play();
								if (tipTimeWarn.opened)
									tipTimeWarn.close();
								tipTimeWarn.showTimed(18000, 0);
							}
						break;
						case 2:
							if (timeLeft <= 1) {
								tipTimeWarn.displayMin = timeLeft.toString();
								playSound.loops = 4;
								playSound.play();
								tipTimeWarn.showTimed(60000, 0);
								stop();
								complete = true;
							}
						break;
					}
				}
			}
		}

		function init(finalTime) {
			bDayIsFinished = false;
			finalHour = runCmd.getHourOrMinutesFromStrTime(finalTime);
			finalMin = runCmd.getMinutesOrSeconsFromStrTime(finalTime);
			tipTimeWarn.nShow = 0;
			timeIn = runCmd.getCurrentTimeString();
			complete = false;
			start();
		}

		function stopTimer() {
			stop();
			playSound.stop();
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
				font.pixelSize: AppSettings.fontSizeTitle
				color: "white"
			}

			GridLayout {
				Layout.fillWidth: true
				columns: 2
				rows: 2
				Layout.leftMargin: 5
				Layout.rightMargin: 5

				Label {
					text: qsTr("Training Division:")
					color: "white"
					font.pixelSize: AppSettings.fontSizeText
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
					color: "white"
					font.pixelSize: AppSettings.fontSizeText
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

			CheckBox {
				id: chkAdjustCalendar
				text: qsTr("Re-adjust meso calendar from this day?")
				checked: false
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				Layout.fillWidth: true
				visible: false
				padding: 0
				spacing: 0

				indicator: Rectangle {
					implicitWidth: 26
					implicitHeight: 26
					x: chkAdjustCalendar.leftPadding
					y: (parent.height-height) / 2
					radius: 5
					border.color: chkAdjustCalendar.down ? primaryDarkColor : primaryLightColor

					Rectangle {
						width: 14
						height: 14
						x: 6
						y: 6
						radius: 2
						color: chkAdjustCalendar.down ? primaryDarkColor : primaryLightColor
						visible: chkAdjustCalendar.checked
					}
				}

				contentItem: Label {
					text: chkAdjustCalendar.text
					color: "white"
					font.pixelSize: AppSettings.fontSizeText
					font.bold: true
					wrapMode: Text.WordWrap
					opacity: enabled ? 1.0 : 0.3
					verticalAlignment: Text.AlignVCenter
					leftPadding: chkAdjustCalendar.indicator.width + chkAdjustCalendar.spacing
				}
			} //CheckBox

			Label {
				text: qsTr("Location:")
				color: "white"
				font.pixelSize: AppSettings.fontSizeText
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

				onTextEdited: {
					location = text;
					tDayModel.modified = true;
				}
			}

			Frame {
				id: frmTrainingTime
				visible: splitLetter !== 'R'
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 20

				background: Rectangle {
					border.color: "white"
					color: "transparent"
					radius: 6
				}

				ColumnLayout {
					anchors.fill: parent

					TPRadioButton {
						id: optFreeTimeSession
						text: qsTr("Open time training session")
						checked: true
						enabled: !timerRestricted.running
						Layout.fillWidth: true
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

						ButtonFlat {
							id: btnTimeLength
							text: qsTr("By duration")
							enabled: !timerRestricted.running
							visible: optTimeConstrainedSession.checked
							Layout.alignment: Qt.AlignCenter
							onClicked: dlgSessionLength.open();
						}
						ButtonFlat {
							id: btnTimeHour
							text: qsTr("By time of day")
							enabled: !timerRestricted.running
							visible: optTimeConstrainedSession.checked
							Layout.alignment: Qt.AlignCenter
							onClicked: dlgTimeEndSession.open();
						}
					} //RowLayout

					Label {
						id: lblTimeRestrictedSession
						text: timerRestricted.running ? qsTr("Alarm will be set to go off in <b>") + runCmd.getStrHourFromTime(sessionLength) + qsTr(" hour(s) and ") +
									runCmd.getStrMinFromTime(sessionLength) + qsTr(" minute(s)</b>, at <b>") + timeOut + "</b>" :
									qsTr("Session time elapsed!")
						wrapMode: Text.WordWrap
						color: "white"
						font.pixelSize: AppSettings.fontSizeText
						font.bold: true
						Layout.fillWidth: true
						Layout.maximumWidth: frmTrainingTime.width
						Layout.rightMargin: 5
						Layout.alignment: Qt.AlignCenter
						visible: timerRestricted.running || timerRestricted.complete
					}

					ButtonFlat {
						id: btnCancelTimeRestrictedTimer
						text: qsTr("Unset alarm")
						Layout.alignment: Qt.AlignCenter
						visible: timerRestricted.running

						onClicked: {
							timerRestricted.stopTimer();
						}
					}

					RowLayout {
						Layout.fillWidth: true
						Layout.leftMargin: 5

						Label {
							id: lblInTime
							color: "white"
							font.pixelSize: AppSettings.fontSizeText
							font.bold: true
							text: qsTr("In time:")
						}
						TPTextInput {
							id: txtInTime
							text: timeIn
							readOnly: true
							Layout.leftMargin: 5

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
							icon.source: "qrc:/images/"+darkIconFolder+"time.png"
							enabled: !timerRestricted.running

							onClicked: dlgTimeIn.open();
						}
					} //RowLayout

					RowLayout {
						Layout.fillWidth: true
						Layout.leftMargin: 5

						Label {
							id: lblOutTime
							color: "white"
							font.pixelSize: AppSettings.fontSizeText
							font.bold: true
							text: qsTr("Out time:")
						}
						TPTextInput {
							id: txtOutTime
							text: timeOut
							readOnly: true
							Layout.leftMargin: 5

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
							icon.source: "qrc:/images/"+darkIconFolder+"time.png"
							enabled: !timerRestricted.running

							onClicked: dlgTimeOut.open();
						}
					} // RowLayout

					Label {
						id: lblSessionLength
						text: qsTr("Total session length: <b>") + runCmd.getStrHourFromTime(sessionLength) + qsTr(" hour(s) and ") +
									runCmd.getStrMinFromTime(sessionLength) + qsTr(" minute(s)</b>")
						wrapMode: Text.WordWrap
						color: "white"
						font.pixelSize: AppSettings.fontSizeText
						font.bold: true
						Layout.fillWidth: true
						Layout.maximumWidth: frmTrainingTime.width
						visible: bDayIsFinished
					}
				} //ColumnLayout
			} //Frame

			Label {
				id: lblDayInfoTrainingNotes
				text: qsTr("This training session considerations:")
				font.pixelSize: AppSettings.fontSizeText
				font.bold: true
				visible: splitLetter !== 'R'
				color: "white"
				Layout.leftMargin: 5

				RoundButton {
					id: btnShowHideNotes
					anchors.left: parent.right
					anchors.verticalCenter: parent.verticalCenter
					anchors.rightMargin: 20
					width: 25
					height: 25

					Image {
						id: img
						source: "qrc:/images/"+darkIconFolder+"down.png"
						width: 20
						height: 20
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
					}

					onClicked: {
						trainingNotesField.visible = !trainingNotesField.visible;
						img.source = trainingNotesField.visible ? "qrc:/images/"+darkIconFolder+"up.png" : "qrc:/images/"+darkIconFolder+"down.png"
					}
				}
			}
			Flickable {
				id: trainingNotesField
				Layout.fillWidth: true
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				height: Math.min(contentHeight, 60)
				contentHeight: txtDayInfoTrainingNotes.implicitHeight
				visible: false

				TextArea.flickable: TextArea {
					id: txtDayInfoTrainingNotes
					text: tDayModel.dayNotes()
					font.bold: true
					font.pixelSize: AppSettings.fontSizeText
					color: "white"

					onEditingFinished: tDayModel.setDayNotes(text);
				}
				ScrollBar.vertical: ScrollBar {}
				ScrollBar.horizontal: ScrollBar {}
			} //Flickable

			Frame {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 20
				visible: bDayIsFinished

				background: Rectangle {
					border.color: "white"
					color: "transparent"
					radius: 6
				}

				ColumnLayout {
					anchors.fill: parent
					spacing: 0

					Label {
						text: qsTr("Replace exercises plan for this division with this day's training list?")
						wrapMode: Text.WordWrap
						font.pixelSize: AppSettings.fontSizeText
						font.bold: true
						Layout.topMargin: 20
						Layout.leftMargin: 5
						color: "white"
						Layout.fillWidth: true
						width: parent.width - 5
						Layout.bottomMargin: 2
						padding: 0
					}

					ButtonFlat {
						id: btnConvertToExercisePlanner
						text: qsTr("Go")
						Layout.alignment: Qt.AlignCenter
						Layout.bottomMargin: 20
						onClicked: convertDayToPlan();
					}
				}
			} //Frame

			Label {
				id: lblExercisesStart
				text: qsTr("--- EXERCISES ---")
				color: "white"
				font.weight: Font.Black
				font.pixelSize: AppSettings.fontSizeTitle
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
					icon.source: "qrc:/images/"+darkIconFolder+"revert-day.png"
					ToolTip.text: "Remove all exercises"

					onClicked: msgClearExercises.init(1);
				}
			}

			GroupBox {
				id: grpIntent
				Layout.fillWidth: true
				Layout.rightMargin: 10
				Layout.leftMargin: 5
				Layout.bottomMargin: 30
				visible: bHasMesoPlan || bHasPreviousTDays
				width: parent.width - 20
				spacing: 0
				padding: 0
				property int option
				property bool highlight: false

				onHighlightChanged: {
					if (highlight) {
						recIntent.border.width = 2;
						anim.start();
					}
					else {
						recIntent.border.width = 1;
						anim.stop();
					}
				}

				onOptionChanged: btnChooseIntent.enabled = true;

				label: Label {
					text: qsTr("What do you want to do today?")
					color: "white"
					font.pixelSize: AppSettings.fontSizeText
					font.bold: true
					anchors.horizontalCenter: parent.horizontalCenter
					anchors.bottomMargin: 10
				}

				background: Rectangle {
					id: recIntent
					color: "transparent"
					border.color: "white"
					radius: 6
				}

				SequentialAnimation {
					id: anim
					loops: Animation.Infinite

					ColorAnimation {
						target: recIntent
						property: "border.color"
						from: "white"
						to: "gold"
						duration: 300
						easing.type: Easing.InOutCubic
					}
					ColorAnimation {
						target: recIntent
						property: "border.color"
						from: "gold"
						to: "white"
						duration: 300
						easing.type: Easing.InOutCubic
					}
				}

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

						onClicked: {
							grpIntent.option = 1;
						}
					}

					TPRadioButton {
						id: optPreviousDay
						text: qsTr("Base this session off the one from the one the days in the list below")
						visible: bHasPreviousTDays
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						onClicked: {
							grpIntent.option = 2;
						}
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

						onClicked: {
							grpIntent.option = 3;
						}
					}

					ButtonFlat {
						id: btnChooseIntent
						text: qsTr("Begin")
						enabled: false
						Layout.alignment: Qt.AlignCenter

						onClicked: {
							highlight = false;

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

		ColumnLayout {
			id: colExercises
			objectName: "tDayExercisesLayout"
			width: parent.width

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
			mainwindow.appAboutToBeSuspended.connect(aboutToBeSuspended);
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
		height: 55
		visible: !exercisesPane.shown

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		ButtonFlat {
			id: btnSaveDay
			enabled: tDayModel.modified
			text: qsTr("Log Day")
			imageSource: "qrc:/images/"+lightIconFolder+"save-day.png"
			textUnderIcon: true
			anchors.left: parent.left
			anchors.leftMargin: 5
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				if (tDayModel.id() === -1) {
					var id;
					function continueSave(_id) {
						if (_id === id) {
							appDB.databaseReady.disconnect(continueSave);
							appDB.updateTrainingDayExercises();
						}
					}
					tDayModel.setMesoId(mesoId);
					tDayModel.setDate(mainDate);
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

		ButtonFlat {
			id: btnAddExercise
			text: qsTr("Add exercise")
			enabled: splitLetter !== 'R' && !grpIntent.visible
			imageSource: "qrc:/images/"+lightIconFolder+"exercises-add.png"
			textUnderIcon: true
			anchors.right: parent.right
			anchors.rightMargin: 5
			anchors.verticalCenter: parent.verticalCenter

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
					appStackView.push(object, StackView.DontLoad);
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
			object.bNewExercise = true;
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
		if (exerciseObjIdx === tDayModel.exercisesNumber() - 1)
			scrollBarPosition = phantomItem.y;
		else
			scrollBarPosition = phantomItem.y - lblExercisesStart.y + setObject.y + setObject.height;
		scrollTraining.scrollToPos(scrollBarPosition);
		bounceTimer.start();
	}

	function createFloatingAddSetButton(exerciseIdx, settype) {
		var component = Qt.createComponent("FloatingButton.qml", Qt.Asynchronous);
		function finishCreation() {
			btnFloat = component.createObject(trainingDayPage, { text:qsTr("Add set"),
					image:"add-new.png", exerciseIdx:exerciseIdx, tDayModel:tDayModel, comboIndex:settype });
			btnFloat.updateDisplayText();
			btnFloat.buttonClicked.connect(createNewSet);
			btnFloat.visible = true;
		}
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function requestFloatingButton(exerciseIdx: int, settype: int) {
		if (btnFloat === null)
			createFloatingAddSetButton(exerciseIdx, settype);
		else {
			btnFloat.exerciseIdx = exerciseIdx;
			btnFloat.comboIndex = settype;
			btnFloat.updateDisplayText();
			btnFloat.visible = true;
		}
	}

	function createNewSet(settype, exerciseidx) {
		itemManager.getExerciseObject(exerciseidx).createSetObject(settype, tDayModel.setsNumber(exercise_idx), exerciseidx, "", "");
	}

	function requestSimpleExercisesList(object, visible, multipleSel) {
		bShowSimpleExercisesList = visible;
		bEnableMultipleSelection = multipleSel;
		itemThatRequestedSimpleList = visible ? object : null;
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
