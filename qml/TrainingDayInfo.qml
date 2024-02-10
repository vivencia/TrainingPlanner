import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtMultimedia

import "jsfunctions.js" as JSF

Page {
	id: trainingDayPage
	title: "trainingPage"

	required property date mainDate //dayDate
	required property int tDay //dayNumber
	required property string splitLetter //daySplitLetter
	required property string mesoName

	property int dayId: -1
	property int mesoId
	property string exercisesNames
	property string timeIn
	property string timeOut
	property string location
	property string trainingNotes

	property string sessionLength
	property string filterString: ""
	property bool bModified: false
	property var exerciseSpriteList: []
	property var setsToBeRemoved: []
	property var mesoSplit
	property var mesoSplitLetter
	property var mesoTDay
	property bool bRealMeso: true
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
	property var timerDialog: null
	property var timerDialogRequester: null

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

	onBModifiedChanged: {
		bNavButtonsEnabled = !bModified;	
	}

	onBDayIsFinishedChanged : {
		if (bDayIsFinished)
			sessionLength = JSF.calculateTimeBetweenTimes(timeIn, timeOut);
	}

	ListModel {
		id: cboModel
	}

	TimePicker {
		id: dlgTimeIn
		hrsDisplay: JSF.getHourOrMinutesFromStrTime (txtInTime.text)
		minutesDisplay: JSF.getMinutesOrSeconsFromStrTime (txtInTime.text)

		onTimeSet: (hour, minutes) => {
			timeIn = hour + ":" + minutes;
			bModified = true;
			bDayIsFinished = false;
		}
	}

	TimePicker {
		id: dlgTimeOut
		hrsDisplay: JSF.getHourOrMinutesFromStrTime (txtOutTime.text)
		minutesDisplay: JSF.getMinutesOrSeconsFromStrTime (txtOutTime.text)

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
			timeOut = JSF.increaseStringTimeBy(JSF.getTimeStringFromDateTime(todayFull), sessionLength);
			bModified = true;
			timerRestricted.init(timeOut);
		}
	}

	TimePicker {
		id: dlgTimeEndSession
		hrsDisplay: JSF.getHourOrMinutesFromStrTime (JSF.getTimeStringFromDateTime(todayFull))
		minutesDisplay: JSF.getMinutesOrSeconsFromStrTime (JSF.getTimeStringFromDateTime(todayFull))

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

	TPBalloonTip {
		id: timerDlgMessage
		title: qsTr("Attention!")
		message: qsTr("Only one timer window can be opened at a time!")
		imageSource: "qrc:/images/"+darkIconFolder+"time.png"
		button1Text: qsTr("OK")
		highlightMessage: true
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
			const timeNow = JSF.getTimeStringFromDateTime(todayFull);
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
			timeIn = JSF.getTimeStringFromDateTime(todayFull);
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
		contentWidth: availableWidth //stops bouncing to the sides
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
				text: qsTr("Trainning day <b>#") + mesoTDay + qsTr("</b> of <b>") + mesoName + "</b>: <b>" +
					JSF.formatDateToDisplay(mainDate, AppSettings.appLocale) + qsTr("</b> Division: <b>") + mesoSplitLetter + "</b>"
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
						maybeResetPage();
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
					text: tDay
					width: 50
					maximumLength: 3
					validator: IntValidator { bottom: 0; top: 365; }
					inputMethodHints: Qt.ImhDigitsOnly
					readOnly: splitLetter === 'R'
					Layout.row: 1
					Layout.column: 1

					onTextEdited: {
						if ( text !== "") {
							if (parseInt(text) !== mesoTDay) {
								bModified = true;
								tDay = text;
							}
						}
					}
				} //txtTDay
			} //GridLayout

			Frame {
				id: frmMesoSplitAdjust
				Layout.fillWidth: true
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				visible: bRealMeso && (splitLetter !== mesoSplitLetter || tDay !== mesoTDay)
				padding: 0
				spacing: 0

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
						Layout.maximumWidth: frmMesoSplitAdjust.width
						Layout.leftMargin: 5

						indicator: Rectangle {
							implicitWidth: 26
							implicitHeight: 26
							x: chkAdjustCalendar.leftPadding
							y: parent.height / 2 - height / 2
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
				text: location
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
						text: timerRestricted.running ? qsTr("Alarm will be set to go off in <b>") + JSF.getHourOrMinutesFromStrTime(sessionLength) + qsTr(" hour(s) and ") +
									JSF.getMinutesOrSeconsFromStrTime(sessionLength) + qsTr(" minute(s)</b>, at <b>") + timeOut + "</b>" :
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
							text: timeIn !== "" ? timeIn : JSF.getTimeStringFromDateTime(todayFull)
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
							text: timeOut !== "" ? timeOut : JSF.getTimeStringFromDateTime(todayFull)
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
						text: qsTr("Total session length: <b>") + JSF.getHourOrMinutesFromStrTime(sessionLength) + qsTr(" hour(s) and ") +
									JSF.getMinutesOrSeconsFromStrTime(sessionLength) + qsTr(" minute(s)</b>")
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
					text: trainingNotes
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
						text: qsTr("Base this session off the one from ") + JSF.formatDateToDisplay(previousDivisionDayDate, AppSettings.appLocale)
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
							if (mesoSplitLetter !== splitLetter) {
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
		if (timerDialog !== null)
			timerDialog.destroy();
		if (navButtons !== null)
			navButtons.destroy();
	}

	Component.onCompleted: {
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

		cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(splitLetter);
	}

	onSplitLetterChanged: {
		const meso_div_info = Database.getDivisionForMeso(mesoId);
		filterString = "";
		if (meso_div_info.lengh > 0) {
			var splitText = "";
			switch (splitLetter) {
				case 'A': splitText = meso_div_info[0].splitA; break;
				case 'B': splitText = meso_div_info[0].splitB; break;
				case 'C': splitText = meso_div_info[0].splitC; break;
				case 'D': splitText = meso_div_info[0].splitD; break;
				case 'E': splitText = meso_div_info[0].splitE; break;
				case 'F': splitText = meso_div_info[0].splitF; break;
				default: return;
			}
			filterString = JSF.makeFilterString(splitText, AppSettings.appLocale);
		}
	}

	function maybeResetPage() {
		checkIfMesoPlanExists();
		checkIfPreviousDayExists();
	}

	function loadOrCreateDayInfo() {
		mesoSplitLetter = splitLetter;
		mesoTDay = tDay;
		let mesoinfo = Database.getMesoInfo(mesoId);
		if ( mesoinfo.length > 0 ) {
			mesoSplit = mesoinfo[0].mesoSplit;
			bRealMeso = mesoinfo[0].realMeso;
			changeComboModel();
		}
		bDayIsFinished = loadTrainingDayInfo(mainDate);
		if (!bDayIsFinished) {
			bLongTask = false;
			checkIfMesoPlanExists();
			checkIfPreviousDayExists();
		}
	}

	function createDatabaseEntryForDay() {
		let result = Database.newTrainingDay(mainDate.getTime(), mesoId, exercisesNames,
			tDay, splitLetter, timeIn, timeOut, location, trainingNotes);
		dayId = result.insertId;
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

	function gotExercise(strName1, strName2, sets, reps, weight) {
		strName1 += ' - ' + strName2;
		addExercise(strName1);
		strName2 = "";
		if (bFirstTime && firstTimeTip)
			firstTimeTip.visible = false;

		var component = Qt.createComponent("ExerciseEntry.qml", Qt.Asynchronous);

		function finishCreation() {
			var idx = exerciseSpriteList.length;
			var exerciseSprite = component.createObject(colExercises, { thisObjectIdx:idx, exerciseName:strName1,
				exerciseName1:strName1, exerciseName2:strName2, splitLetter:splitLetter, tDayId:dayId });
			exerciseSprite.exerciseRemoved.connect(removeExercise);
			exerciseSprite.exerciseEdited.connect(editExercise);
			exerciseSprite.setAdded.connect(addExerciseSet);
			exerciseSprite.setWasRemoved.connect(delExerciseSet);
			exerciseSprite.requestHideFloatingButtons.connect(hideFloatingButton);
			exerciseSpriteList.push({"Object" : exerciseSprite});

			bStopBounce = true;
			if (navButtons === null)
				createNavButtons();
			scrollBarPosition = phantomItem.y;
			scrollTraining.scrollToPos(scrollBarPosition);
			bounceTimer.start();
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
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

	function editExercise(exerciseIdx, newExerciseName) {
		bModified = true;
		if (exerciseIdx !== -1) {
			const names = exercisesNames.split('|');
			exercisesNames = "";
			for (var i = 0; i < names.length; ++i) {
				if (i !== exerciseIdx)
					exercisesNames += names[i] + '|';
				else
					exercisesNames += newExerciseName + '|';
			}
			exercisesNames = exercisesNames.slice(0, -1);
		}
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

	function updateMesoCalendar() {
		if (mesoSplitLetter !== splitLetter || mesoTDay !== tDay) {
			var calendar_date_info;
			var cal_id;
			calendar_date_info = Database.getMesoCalendarDate(mainDate);
			if (calendar_date_info.length > 0) {
				cal_id = calendar_date_info[0].mesoCalId;
				Database.updateMesoCalendarDaySplit(cal_id, splitLetter);

				//If in the old calendar, day was R or the user did not explicitly change tDay, go back
				//in date until we find a suitable training day. This will be starting training day if the user
				//wants to modify the rest of the calendar, or simply the continuation from the previous training day if not
				if (mesoSplitLetter === 'R' || tDay === mesoTDay) {
					//Find a suitable training day to continue
					var prevdate = JSF.getPreviousDate(mainDate);
					//Find the first training day before mainDate that is not a rest day;
					while ( (calendar_date_info = Database.getMesoCalendarDate(prevdate)) !== null) {
						//console.log(prevdate.toDateString());
						//console.log(calendar_date_info[0].mesoCalSplit);
						if (calendar_date_info[0].mesoCalSplit !== 'R') {
							tDay = calendar_date_info[0].mesoCalnDay + 1;
							break;
						}
						prevdate = JSF.getPreviousDate(prevdate);
					}
				}
				//Update this day
				Database.updateMesoCalendarTrainingDay(cal_id, tDay);
				if (chkAdjustCalendar.checked) {
					var split_idx;
					var tday;
					var splitletter;
					if (optUpdateCalendarContinue.checked) {
						split_idx = mesoSplit.indexOf(splitLetter);
						split_idx++;
						if (split_idx >= mesoSplit.length)
							split_idx = 0;

						tday = tDay + 1; //tDay of the next day in calendar
					}
					else { //Start all over
						Database.updateMesoCalendarTrainingDay(cal_id, splitLetter === 'R' ? 0 : 1); //Update this day
						split_idx = 0;
						tday = splitLetter === 'R' ? 1 : 2; //tDay of the next day in calendar
					}

					let result = Database.getMesoInfo(mesoId);
					if (result.length === 1) {
						const mesoenddate = new Date(result[0].mesoEndDate);
						startProgressDialog(cal_id, split_idx, tday, mesoenddate);
					}
				} // chkAdjustCalendar.checked = true

				//Hide the frame for adjustments
				mesoSplitLetter = splitLetter;
				mesoTDay = tDay;
				mesoCalendarChanged(); //Signals MesoContent.qml to reread from the database
			} //calendar_date_info.length > 0
		} //mesoSplitLetter !== splitLetter
	} // updateMesoCalendar()

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
				if (location === "")
					location = txtLocation.placeholderText;
				if (trainingNotes === "")
					trainingNotes = " ";
				if (splitLetter === 'R')
					tDay = 0;
				if (bRealMeso)
					updateMesoCalendar();
				if (dayId === -1) {
					createDatabaseEntryForDay();
					updateDayIdFromExercisesAndSets();
				}
				else
					Database.updateTrainingDay(dayId, exercisesNames, tDay, splitLetter, timeIn, timeOut, location, trainingNotes);

				var i = 0;
				for (; i < setsToBeRemoved.length; ++i)
					Database.deleteSetFromSetsInfo(setsToBeRemoved[i]);
				setsToBeRemoved = [];
				const len = exerciseSpriteList.length;
				for (i = 0; i < exerciseSpriteList.length; ++i)
					exerciseSpriteList[i].Object.logSets();
				bModified = false;
				appDBModified = true;
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

				openDbExercisesListPage();
				dbExercisesListPage.bChooseButtonEnabled = true;
				dbExercisesListPage.exerciseChosen.connect(gotExercise);
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
				exercisesList.setFilter(filterString);
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
		else {
			timerDlgMessage.openTimed(5000, 0);
		}
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

	function aboutToBeSuspended() {
		if (bModified)
			btnSaveDay.clicked();
	}
} // Page
