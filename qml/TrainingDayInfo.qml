import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtMultimedia

import com.vivenciasoftware.qmlcomponents

Page {
	id: trainingDayPage
	title: "trainingPage"
	width: windowWidth
	//height: parentItem.height

	required property date mainDate //dayDate
	required property int mesoId
	required property int mesoIdx
	required property int modelIdx
	required property DBTrainingDayModel tDayModel

	property int dayId: -1
	property string exercisesNames
	property string tDay
	property string splitLetter
	property string timeIn
	property string timeOut
	property string location
	property string trainingNotes

	property string mesoSplit
	property string mesoName
	property bool bRealMeso: true


	property date sessionLength
	property string filterString: ""
	property bool bModified: false
	property var exerciseSpriteList: []
	property var setsToBeRemoved: []

	property bool bFirstTime: false
	property bool bAlreadyLoaded
	property int totalNumberOfExercises

	property bool bStopBounce: false
	property bool bNotScroll: true
	property bool bHasPreviousDay: false
	property bool bHasMesoPlan: false
	property bool bDayIsFinished: false
	property date previousDivisionDayDate
	property int scrollBarPosition: 0
	property var navButtons: null
	property var firstTimeTip: null

	property bool bShowSimpleExercisesList: false
	property var exerciseEntryThatRequestedSimpleList: null

	signal mesoCalendarChanged()

	property var splitModel: [ { value:'A', text:'A' }, { value:'B', text:'B' }, { value:'C', text:'C' },
							{ value:'D', text:'D' }, { value:'E', text:'E' }, { value:'F', text:'F' }, { value:'R', text:'R' } ]

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

	onTotalNumberOfExercisesChanged: {
		if (totalNumberOfExercises <= 0) {
			bLongTask = false;
			if (bHasPreviousDay || bHasMesoPlan) {
				bHasPreviousDay = bHasMesoPlan = false;
				dayId = -1;
			}
			scrollTraining.setScrollBarPosition(1); //Scroll to bottom
			createNavButtons();
		}
	}

	//onBModifiedChanged: {
	//	bNavButtonsEnabled = !bModified;
	//}

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
			bModified = true;
			bDayIsFinished = false;
		}
	}

	TimePicker {
		id: dlgTimeOut
		hrsDisplay: runCmd.getHourOrMinutesFromStrTime(txtOutTime.text)
		minutesDisplay: runCmd.getMinutesOrSeconsFromStrTime(txtOutTime.text)

		onTimeSet: (hour, minutes) => {
			timeOut = hour + ":" + minutes;
			bModified = true;
			hideFloatingButton(-1); //Day is finished
			bDayIsFinished = true;
			btnSaveDay.clicked();
			foldUpAllExercisesEntries("up");
		}
	}

	TimerDialog {
		id: dlgSessionLength
		timePickerOnly: true
		windowTitle: qsTr("Length of this training session")

		onUseTime: (strtime) => {
			sessionLength = strtime;
			timeOut = runCmd.formatFutureTime(mainwindow.todayFull, sessionLength);
			bModified = true;
			timerRestricted.init(timeOut);
		}
	}

	TimePicker {
		id: dlgTimeEndSession
		hrsDisplay: runCmd.getStrHourFromTime(mainwindow.todayFull)
		minutesDisplay: runCmd.getStrMinFromTime(mainwindow.todayFull)

		onTimeSet: (hour, minutes) => {
			timeOut = hour + ":" + minutes;
			bModified = true;
			timerRestricted.init(timeOut);
		}
	}

	SoundEffect {
		id: playSound
		source: "qrc:/sounds/timer-end.wav"
	}

	TPBalloonTip {
		id: tipTimeWarn
		title: qsTr("Attention!")
		message: "</b>" + displayMin + qsTr("</b> minute(s) until end of training session!")
		imageSource: "qrc:/images/"+darkIconFolder+"sound-off.png"
		button1Text: qsTr("OK")

		property string displayMin
		property int nShow: 0

		onOpened: nShow++;
		onButton1Clicked: playSound.stop();
	}

	Timer {
		id: loadTimer
		interval: 300
		running: false
		repeat: false
		property int mOpt: 0

		onTriggered: {
			switch (mOpt) {
				case 0:	loadTrainingDayInfoFromMesoPlan(); break;
				case 1: loadTrainingDayInfo(previousDivisionDayDate); break;
				case 2: loadOrCreateDayInfo(); break;
			}
		}

		function init(opt) {
			mOpt = opt;
			bLongTask = true;
			busyIndicator.visible = true;
			start();
		}
	}

	Timer {
		id: timerRestricted
		interval: 20000 //Every twenty seconds
		repeat: true
		property bool complete: false
		property string finalHour
		property string finalMin

		onTriggered: {
			const timeNow = JSF.getTimeStringFromDateTime(mainwindow.todayFull);
			sessionLength = JSF.calculateTimeBetweenTimes(timeNow, timeOut);

			const hour = JSF.getHourOrMinutesFromStrTime(timeNow);
			if (hour === finalHour) {
				const min = JSF.getMinutesOrSeconsFromStrTime(timeNow);
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
						case 2: {
							if (timeLeft <= 1) {
								tipTimeWarn.displayMin = timeLeft.toString();
								playSound.loops = 4;
								playSound.play();
								tipTimeWarn.showTimed(60000, 0);
								stop();
								complete = true;
							}
						}
					}
				}
			}
		}

		function init(finalTime) {
			bDayIsFinished = false;
			finalHour = JSF.getHourOrMinutesFromStrTime(finalTime);
			finalMin = JSF.getMinutesOrSeconsFromStrTime(finalTime);
			tipTimeWarn.nShow = 0;
			tipTimeWarn.timeout = 20000;
			timeIn = JSF.getTimeStringFromDateTime(mainwindow.todayFull);
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
				text: qsTr("Trainning day <b>#") + tDayModel.trainingDay() + qsTr("</b> of <b>") + mesoName + "</b>: <b>" +
					runCmd.formatDate(mainDate) + qsTr("</b> Division: <b>") + tDayModel.splitLetter() + "</b>"
				font.pixelSize: AppSettings.titleFontSizePixelSize
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
						bModified = true;
						splitLetter = cboModel.get(index).value;
						if (splitLetter === 'R')
							tDay = "0";
						else
							tDay = txtTDay.text;
						//maybeResetPage();
					}			
				} //TPComboBox

				Label {
					text: qsTr("Training Day #")
					color: "white"
					font.pixelSize: AppSettings.fontSizeText
					font.bold: true
					Layout.row: 1
					Layout.column: 0
				}
				TPTextInput {
					id: txtTDay
					text: tDayModel.trainingDay() === "" ? tDay : tDayModel.trainingDay()
					width: 50
					maximumLength: 3
					validator: IntValidator { bottom: 0; top: 365; }
					inputMethodHints: Qt.ImhDigitsOnly
					readOnly: splitLetter === 'R'
					Layout.row: 1
					Layout.column: 1

					onTextEdited: {
						if ( text !== "") {
							if (text !== tDayModel.trainingDay()) {
								bModified = true;
								tDay = text;
							}
						}
					}
				} //txtTDay
			} //GridLayout

			Frame {
				id: frmMesoSplitAdjust
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				visible: bRealMeso && (splitLetter !== tDayModel.splitLetter() || tDay !== tDayModel.trainingDay())
				padding: 0
				spacing: 0
				width: windowWidth - 20
				height: 200

				background: Rectangle {
					border.color: "white"
					color: "transparent"
					radius: 6
				}

				ColumnLayout {
					id: layoutSplit
					anchors.fill: parent
					spacing: 0

					CheckBox {
						id: chkAdjustCalendar
						text: qsTr("Adjust meso calendar from the next day till the end?")
						checked: false
						Layout.maximumWidth: parent.width - 10
						Layout.leftMargin: 5

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

						onClicked: {
							if (checked)
								optUpdateCalendarContinue.checked = true;
						}
					} //CheckBox

					TPRadioButton {
						id: optUpdateCalendarContinue
						text: qsTr("Continue cycle from this division letter")
						enabled: chkAdjustCalendar.checked
						checked: false
					}

					TPRadioButton {
						id: optUpdateCalendarStartOver
						text: qsTr("Start cycle over")
						checked: false
						enabled: chkAdjustCalendar.checked
					}
				} // ColumnLayout
			} //Frame

			Label {
				text: qsTr("Location:")
				color: "white"
				font.pixelSize: AppSettings.fontSizeText
				font.bold: true
				Layout.leftMargin: 5
			}
			TPTextInput {
				id: txtLocation
				placeholderText: "Academia Golden Era"
				text: tDayModel.location()
				Layout.fillWidth: true
				Layout.rightMargin: 10
				Layout.leftMargin: 5

				onTextEdited: {
					if (text.length >= 4) {
						bModified = true;
						location = text;
					}
				}
			}

			Frame {
				id: frmTrainingTime
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
							text: timeIn !== "" ? timeIn : runCmd.formatTime(mainwindow.todayFull)
							readOnly: true
							Layout.leftMargin: 5
						}
						RoundButton {
							id: btnInTime
							icon.source: "qrc:/images/"+darkIconFolder+"time.png"

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
							text: timeOut !== "" ? timeOut : runCmd.formatFutureTime(mainwindow.todayFull, 1, 30)
							readOnly: true
							Layout.leftMargin: 5
						}

						RoundButton {
							id: btnOutTime
							icon.source: "qrc:/images/"+darkIconFolder+"time.png"

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
				color: "white"
				Layout.leftMargin: 5
			}
			Flickable {
				Layout.fillWidth: true
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				height: Math.min(contentHeight, 60)
				contentHeight: txtDayInfoTrainingNotes.implicitHeight

				TextArea.flickable: TextArea {
					id: txtDayInfoTrainingNotes
					text: tDayModel.dayNotes()
					font.bold: true
					font.pixelSize: AppSettings.fontSizeText
					color: "white"

					onEditingFinished: {
						if (text.length >= 4) {
							bModified = true;
							trainingNotes = text;
						}
					}
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
				font.pixelSize: AppSettings.titleFontSizePixelSize
				Layout.alignment: Qt.AlignCenter
				Layout.bottomMargin: 2
			}

			GroupBox {
				id: grpIntent
				Layout.fillWidth: true
				Layout.rightMargin: 10
				Layout.leftMargin: 5
				Layout.bottomMargin: 30
				visible: bHasMesoPlan || bHasPreviousDay
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
					anchors.fill: parent
					spacing: 0

					TPRadioButton {
						id: optMesoPlan
						text: qsTr("Use the standard exercises plan for the division ") + splitLetter + qsTr(" of the Mesocycle")
						visible: bHasMesoPlan
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						onClicked: {
							grpIntent.option = 0;
						}
					}
					TPRadioButton {
						id: optPreviousDay
						text: qsTr("Base this session off the one from ") + runCmd.formatDate(previousDivisionDayDate)
						visible: bHasPreviousDay
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						onClicked: {
							grpIntent.option = 1;
						}
					}
					TPRadioButton {
						id: optEmptySession
						text: qsTr("Start an empty session")
						visible: bHasMesoPlan || bHasPreviousDay
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						onClicked: {
							grpIntent.option = 2;
						}
					}

					TPRadioButton {
						id: optContinueSession
						text: qsTr("Just continue with the changes already made")
						visible: bModified && (bHasMesoPlan || bHasPreviousDay)
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
						Layout.alignment: Qt.AlignCenter

						onClicked: {
							highlight = false;
							if (tDayModel.splitLetter() !== splitLetter) {
								if (exercisesNames.length > 0) {
									if (grpIntent.option !== 3)
										removeAllExerciseObjects();
								}
							}

							switch (grpIntent.option) {
								case 0: //use meso plan
									loadTimer.init(0);
								break;
								case 1: //use previous day
									loadTimer.init(1);
								break;
								case 2: //empty session
								case 3: //continue session
									bHasPreviousDay = false;
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
			width: parent.width

			anchors {
				left: parent.left
				leftMargin: 5
				rightMargin: 5
				right:parent.right
				top: colMain.bottom
			}

			/*DBTrainingDayModel {
				id: tModel
			}

			ExerciseEntry {
				id: exercise1
				thisObjectIdx: 0
				tDayModel: tModel
			}*/
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
			bNotScroll = true;
			if (pos === 0)
				ScrollBar.vertical.setPosition(0);
			else
				ScrollBar.vertical.setPosition(pos - ScrollBar.vertical.size/2);
		}
	} // ScrollView scrollTraining

	Component.onDestruction: {
		if (navButtons !== null)
			navButtons.destroy();
	}

	Component.onCompleted: {
		console.log("onCompleted")
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
			bNotScroll = false;
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

	onSplitLetterChanged: {
		var splitText;
		switch (splitLetter) {
			case 'A': splitText = mesoSplitModel.get(mesoIdx, 2); break;
			case 'B': splitText = mesoSplitModel.get(mesoIdx, 3); break;
			case 'C': splitText = mesoSplitModel.get(mesoIdx, 4); break;
			case 'D': splitText = mesoSplitModel.get(mesoIdx, 5); break;
			case 'E': splitText = mesoSplitModel.get(mesoIdx, 6); break;
			case 'F': splitText = mesoSplitModel.get(mesoIdx, 7); break;
			default: return;
		}
		filterString = exercisesListModel.makeFilterString(splitText);
	}

	function maybeResetPage() {
		checkIfMesoPlanExists();
		checkIfPreviousDayExists();
	}

	function loadOrCreateDayInfo() {
		bDayIsFinished = loadTrainingDayInfo(mainDate);
		if (!bDayIsFinished) {
			bLongTask = false;
			checkIfMesoPlanExists();
			checkIfPreviousDayExists();
		}
	}

	function checkIfMesoPlanExists() {
		switch (splitLetter) {
			case 'A': exercisesNames = Database.getExercisesFromDivisionAForMeso(mesoId); break;
			case 'B': exercisesNames = Database.getExercisesFromDivisionBForMeso(mesoId); break;
			case 'C': exercisesNames = Database.getExercisesFromDivisionCForMeso(mesoId); break;
			case 'D': exercisesNames = Database.getExercisesFromDivisionDForMeso(mesoId); break;
			case 'E': exercisesNames = Database.getExercisesFromDivisionEForMeso(mesoId); break;
			case 'F': exercisesNames = Database.getExercisesFromDivisionFForMeso(mesoId); break;
		}
		bHasMesoPlan = exercisesNames.length > 1;
	}

	function loadTrainingDayInfoFromMesoPlan() {
		createExercisesFromList(false);
		bModified = true;
	}

	function checkIfPreviousDayExists() {
		let day_info = Database.getPreviousTrainingDayForDivision(splitLetter, tDay, mesoId);
		for (var i = 0; i < day_info.length; ++i ) { //from the most recent to the oldest
			if (day_info[i].exercisesNames) {
				bHasPreviousDay = day_info[i].exercisesNames.length > 1;
				if (bHasPreviousDay) {
					previousDivisionDayDate = new Date(day_info[i].dayDate);
					break;
				}
			}
			if (!bHasPreviousDay)
				Database.deleteTraingDay(day_info[i].dayId); //remove empty day from DB
		}
	}

	function loadTrainingDayInfo(tDate) {
		let dayInfoList = Database.getTrainingDay(tDate.getTime());
		if (dayInfoList.length > 0) {
			if (!dayInfoList[0].exercisesNames) {//Day is saved but it is empty. Treat it as if it weren't saved then
				Database.deleteTraingDay(dayInfoList[0].dayId);
				return false;
			}
			dayId = dayInfoList[0].dayId;
			const tempMesoId = dayInfoList[0].mesoId;
			if (tempMesoId !== mesoId) {
				//The information recorded for the day has a mesoId that refers to a mesocycle that does not
				//match the passed mesoId. Since all the ways to get to this page must go through methods that check
				//the valilidy of a mesoId, the stored mesoId is wrong and must be replaced
				Database.updateTrainingDay_MesoId(dayId, mesoId);
			}
			exercisesNames = dayInfoList[0].exercisesNames;
			timeIn = dayInfoList[0].dayTimeIn;
			timeOut = dayInfoList[0].dayTimeOut;
			location = dayInfoList[0].dayLocation;
			trainingNotes = dayInfoList[0].dayNotes;
			createExercisesFromList(true);
			return true;
		}
		return false;
	}

	function createExercisesFromList(bFromList) {
		const names = exercisesNames.split('|');
		var sep, name, name2 = "";
		const len = names.length;
		totalNumberOfExercises = len
		for (var i = 0; i < len; ++i) {
			sep = names[i].indexOf('&');
			if (sep !== -1) { //Composite exercise
				name = names[i].substring(0, sep);
				name2 = names[i].substring(sep + 1, names[i].length);
			}
			else
				name = names[i];

			function generateExerciseObject(nName1, nName2) {
				var component = Qt.createComponent("ExerciseEntry.qml", Component.Asynchronous);

				function finishCreation(Name1, Name2) {
					const idx = exerciseSpriteList.length;
					var exerciseSprite = component.createObject(colExercises, {
							thisObjectIdx:idx, loadObjectIdx:idx, exerciseName:Name1, setBehaviour: bFromList ? 1 : 2,
							exerciseName1:Name1, exerciseName2:Name2, splitLetter:splitLetter, tDayId:dayId, loadTDayId:dayId
					});
					exerciseSprite.exerciseRemoved.connect(removeExercise);
					exerciseSprite.exerciseEdited.connect(editExercise);
					exerciseSprite.setAdded.connect(addExerciseSet);
					exerciseSprite.setWasRemoved.connect(delExerciseSet);
					exerciseSprite.requestHideFloatingButtons.connect(hideFloatingButton);
					exerciseSpriteList.push({"Object" : exerciseSprite});
					totalNumberOfExercises--;
				}

				function checkStatus() {
					if (component.status === Component.Ready)
						finishCreation(nName1, nName2);
				}

				if (component.status === Component.Ready)
					finishCreation(nName1, nName2);
				else
					component.statusChanged.connect(checkStatus);
			}

			generateExerciseObject(name, name2);
		}
	}

	function convertDayToPlan() {
		var exercises = "", types = "", nsets = "", nreps = "",nweights = "";
		var exercisename = ""
		const searchRegExp = /\ \+\ /g;
		const replaceWith = '&';
		const len = exerciseSpriteList.length;
		for (var i = 0; i < len; ++i) {
			exercisename = exerciseSpriteList[i].Object.exerciseName.replace(searchRegExp, replaceWith);
			exercises += exercisename + '|';
			types += exerciseSpriteList[i].Object.setType + '|';
			nsets += exerciseSpriteList[i].Object.setObjectList.length.toString() + '|';
			if (exerciseSpriteList[i].Object.setObjectList[0].Object) {
				nreps += exerciseSpriteList[i].Object.setObjectList[0].Object.setReps.toString() + '|';
				nweights += exerciseSpriteList[i].Object.setObjectList[0].Object.setWeight.toString() + '|';
			}
			else { //no sets for some reason
				nreps += '|';
				nweights += '|';
			}
		}
		const divisionId = Database.getDivisionIdForMeso(mesoId);
		Database.updateMesoDivision_OnlyExercises(divisionId, splitLetter, exercises.slice(0, -1),
				types.slice(0, -1), nsets.slice(0, -1), nreps.slice(0, -1), nweights.slice(0, -1));
		btnConvertToExercisePlanner.enabled = false;
	}

	function updateDayIdFromExercisesAndSets() {
		const len = exerciseSpriteList.length;
		for( var i = 0; i < len; ++i )
			exerciseSpriteList[i].Object.updateDayId(dayId);
	}

	function gotExercise(strName1, strName2) {
		function readyToProceed(object) {
			appDB.getQmlObject.disconnect(readyToProceed);
			object.exerciseRemoved.connect(removeExercise);
			object.exerciseEdited.connect(editExercise);
			object.setAdded.connect(addExerciseSet);
			object.setWasRemoved.connect(delExerciseSet);
			object.requestHideFloatingButtons.connect(hideFloatingButton);

			bStopBounce = true;
			if (navButtons === null)
				createNavButtons();
			scrollBarPosition = phantomItem.y;
			scrollTraining.scrollToPos(scrollBarPosition);
			bounceTimer.start();
			bModified = true;
			return;
		}

		appDB.getQmlObject.connect(readyToProceed);
		appDB.createExerciseObject(strName1 + " - " + strName2, colExercises, modelIdx);
	}

	function addExerciseSet(bnewset, exerciseObjIdx, setObject) {
		if (bnewset) {
			bModified = true;
			bStopBounce = true;
			if (exerciseObjIdx === exerciseSpriteList.length-1)
				scrollBarPosition = phantomItem.y;
			else
				scrollBarPosition = phantomItem.y - lblExercisesStart.y + setObject.y + setObject.height;
			scrollTraining.scrollToPos(scrollBarPosition);
			bounceTimer.start();
		}
	}

	function delExerciseSet(setid) {
		setsToBeRemoved.push(setid);
	}

	function editExercise(exerciseIdx) {
		bModified = true;
	}

	function removeExercise(objidx) {
		let newObjectList = new Array;
		const len = exerciseSpriteList.length;

		for( var i = 0, x = 0; i < len; ++i ) {
			if (i === objidx) {
				removeExerciseName(objidx);
				exerciseSpriteList[objidx].Object.destroy();
			}
			else {
				newObjectList[x] = exerciseSpriteList[i];
				newObjectList[x].Object.updateSetsExerciseIndex(x);
				x++;
			}
		}
		delete exerciseSpriteList;
		exerciseSpriteList = newObjectList;
		bModified = true;
	}

	function addExercise(exercisename) {
		if (exercisesNames.length > 0)
			exercisesNames += '|' + exercisename;
		else
			exercisesNames = exercisename;
		bModified = true;
	}

	function removeExerciseName(exerciseIdx) {
		const names = exercisesNames.split('|');
		exercisesNames = "";
		for (var i = 0; i < names.length; ++i) {
			if (i !== exerciseIdx)
				exercisesNames += names[i] + '|';
		}
		exercisesNames = exercisesNames.slice(0, -1);
		bModified = true;
	}

	function removeAllExerciseObjects() {
		if (navButtons !== null)
			navButtons.visible = false;
		const len = exerciseSpriteList.length - 1;
		for (var i = len; i >= 0; --i)
			exerciseSpriteList[i].Object.destroy();

		exerciseSpriteList = [];
		setsToBeRemoved = [];
		exercisesNames = "";
		bAlreadyLoaded = false;
	}



	function startProgressDialog(calid, splitidx, day, mesoenddate) {
		var component = Qt.createComponent("TrainingDayProgressDialog.qml");

		function finishCreation() {
			var progressSprite = component.createObject(trainingDayPage, {calId:calid, splitIdx:splitidx, tDay:day,
				mesoSplit:mesoSplit, mesoEndDate:mesoenddate
			});
			progressSprite.init(qsTr("Updating calendar database. Please wait..."), 0, 30);
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function hideFloatingButton(except_idx) {
		const len = exerciseSpriteList.length;
		for (var x = 0; x < len; ++x) {
			if (x !== except_idx) {
				if (exerciseSpriteList[x].Object.bFloatButtonVisible)
					exerciseSpriteList[x].Object.bFloatButtonVisible = false;
			}
		}
	}

	function foldUpAllExercisesEntries() {
		const len = exerciseSpriteList.length;
		for (var i = 0; i < len; ++i)
			exerciseSpriteList[i].Object.foldUpSets();
	}

	footer: ToolBar {
		id: dayInfoToolBar
		width: parent.width
		height: 55
		visible: !bShowSimpleExercisesList

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		ButtonFlat {
			id: btnSaveDay
			enabled: bModified
			text: qsTr("Log Day")
			imageSource: "qrc:/images/"+lightIconFolder+"save-day.png"
			textUnderIcon: true
			anchors.left: parent.left
			anchors.leftMargin: 5
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				//if (bRealMeso)
				//	updateMesoCalendar();
				if (dayId === -1) {
					var id;
					function continueSave(_id) {
						if (_id === id) {
							appDB.qmlReady.disconnect(continueSave);
							dayId = appDB.insertId();
							//updateDayIdFromExercisesAndSets();
						}
					}

					id = appDB.pass_object(tDayModel);
					appDB.qmlReady.connect(continueSave);
					appDB.newTrainingDay(mesoId, mainDate, tDay, splitLetter, timeIn, timeOut, location, trainingNotes);
				}
				else {
					appDB.pass_object(tDayModel);
					appDB.updateTrainingDay(dayId, mesoId, mainDate, tDay, splitLetter, timeIn, timeOut, location, trainingNotes);
					appDB.updateTrainingDayExercises(dayId);
				}
				bModified = false;
			}
		} //btnSaveDay

		ButtonFlat {
			id: btnRevertDay
			enabled: bModified
			text: qsTr("Cancel alterations")
			imageSource: "qrc:/images/"+lightIconFolder+"revert-day.png"
			textUnderIcon: true
			anchors.left: btnSaveDay.right
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				bModified = false;
				removeAllExerciseObjects();
				pageActivation();
			}
		} //btnRevertDay

		ButtonFlat {
			id: btnAddExercise
			text: qsTr("Add exercise")
			enabled: !grpIntent.visible
			imageSource: "qrc:/images/"+lightIconFolder+"exercises-add.png"
			textUnderIcon: true
			anchors.right: parent.right
			anchors.rightMargin: 5
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				hideFloatingButton(-1);
				if (navButtons !== null)
					navButtons.hideButtons();

				function pushOntoStackView(object, bfirsttime) {
					if (bfirsttime) {
						object.bChooseButtonEnabled = true;
						object.exerciseChosen.connect(gotExercise);
					}
					appDB.getQmlObject.disconnect(pushOntoStackView);
					mainwindow.appStackView.push(object, StackView.DontLoad);
				}

				appDB.getQmlObject.connect(pushOntoStackView);
				appDB.openExercisesListPage();
			}
		} // bntAddExercise
	} //footer: ToolBar

	ColumnLayout {
		id: bottomPane
		width: parent.width
		spacing: 0
		visible: bShowSimpleExercisesList
		height: shown ? parent.height * 0.5 : btnShowHideList.height

		onVisibleChanged: {
			shown = visible;
		}

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		property bool shown: true

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		onShownChanged: {
			if (shown)
				exercisesListModel.setFilter(filterString);
		}

		ButtonFlat {
			id: btnShowHideList
			imageSource: bottomPane.shown ? "qrc:/images/"+darkIconFolder+"fold-down.png" : "qrc:/images/"+darkIconFolder+"fold-up.png"
			imageSize: 60
			onClicked: bottomPane.shown = !bottomPane.shown;
			Layout.fillWidth: true
			Layout.topMargin: 0
			height: 10
			width: bottomPane.width
		}

		ExercisesListView {
			id: exercisesList
			height: mainwindow.height * 0.8
			Layout.fillWidth: true
			Layout.topMargin: 0
			Layout.alignment: Qt.AlignTop
			Layout.rightMargin: 5
			Layout.maximumHeight: parent.height * 0.8
			Layout.leftMargin: 5
			Layout.fillHeight: true

			onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath, multipleSelection) => {
				if (exerciseEntryThatRequestedSimpleList)
					exerciseEntryThatRequestedSimpleList.changeExercise(exerciseName, subName);
			}
		}
	}

	function requestSimpleExerciseList(object) {
		bShowSimpleExercisesList = true;
		exerciseEntryThatRequestedSimpleList = object;
		scrollTraining.setScrollBarPosition(1);
	}

	function closeSimpleExerciseList() {
		bShowSimpleExercisesList = false;
		exerciseEntryThatRequestedSimpleList = null;
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

	function pageActivation() {
		mesoName = mesocyclesModel.get(mesoIdx, 1);
		mesoSplit = mesocyclesModel.get(mesoIdx, 6);
		bRealMeso = mesocyclesModel.get(mesoIdx, 3) !== "0";
		changeComboModel();
		return;
		if (!bAlreadyLoaded) {
			loadTimer.init(2);
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
		hideFloatingButton(-1);
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

	function aboutToBeSuspended() {
		if (bModified)
			btnSaveDay.clicked();
	}
} // Page
