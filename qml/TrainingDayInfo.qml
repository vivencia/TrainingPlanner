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
	property bool bModified: false
	property var exerciseSpriteList: []
	property var mesoSplit
	property var mesoSplitLetter
	property var mesoTDay
	property bool bRealMeso: true
	property bool bFirstTime: false

	property bool bStopBounce: false
	property bool bNotScroll: true
	property bool bHasPreviousDay: false
	property bool bHasMesoPlan: false
	property date previousDivisionDayDate
	property int scrollBarPosition: 0
	property var navButtons: null
	property var firstTimeTip: null

	property bool bShowSimpleExercisesList: false
	property var exerciseEntryThatRequestedSimpleList: null

	signal mesoCalendarChanged()

	property var splitModel: [ { value:'A', text:'A' }, { value:'B', text:'B' }, { value:'C', text:'C' },
							{ value:'D', text:'D' }, { value:'E', text:'E' }, { value:'F', text:'F' }, { value:'R', text:'R' } ]

	background: Rectangle {
		color: primaryDarkColor
		opacity: 0.7
		Image {
			anchors.fill: parent
			source: "qrc:/images/app_logo.png"
			fillMode: Image.PreserveAspectFit
			opacity: 0.6
		}
	}

	onBModifiedChanged: {
		bNavButtonsEnabled = !bModified;
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
			btnSaveDay.clicked();
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
			sessionLength = JSF.calculateTimeBetweenTimes(JSF.getTimeStringFromDateTime(todayFull), timeOut);
			bModified = true;
			timerRestricted.init(timeOut);
		}
	}

	SoundEffect {
		id: playSound
		source: "qrc:/sounds/timer-end.wav"
	}

	ToolTip {
		id: tipTimeWarn
		text: qsTr("Attention! <b>") + displayMin + qsTr("</b> minute(s) until end of training session!")
		property string displayMin
		property int nShow: 0
		timeout: 18000
		x: 0
		y: 0
		width: mainwindow.width * 0.9
		height: 50
		parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates

		contentItem: Text {
			text: tipTimeWarn.text
			wrapMode: Text.WordWrap
			color: "white"
			font.pixelSize: AppSettings.fontSizeText
			width: parent.width - 40

			anchors {
				left: parent.Left
				leftMargin: 5
				top: parent.top
				topMargin: 5
			}

			RoundButton {
				id: btnMuteSound
				anchors.left: parent.right
				anchors.leftMargin: -width/2
				anchors.verticalCenter: parent.verticalCenter
				onClicked: playSound.stop();

				Image {
					source: "qrc:/images/"+lightIconFolder+"sound-off.png"
					width: 20
					height: 20
					fillMode: Image.PreserveAspectFit
					anchors.verticalCenter: parent.verticalCenter
					anchors.horizontalCenter: parent.horizontalCenter
				}
			}
		} //contentItem

		background: Rectangle {
			color: "black"
			opacity: 0.6
			radius: 8
		}

		onOpened: {
			nShow++;
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
							tipTimeWarn.open();
						break;
						case 1:
							if (timeLeft <= 5) {
								tipTimeWarn.displayMin = timeLeft.toString();
								playSound.play();
								if (tipTimeWarn.opened)
									tipTimeWarn.close();
								tipTimeWarn.open();
							}
						break;
						case 2: {
							if (timeLeft <= 1) {
								tipTimeWarn.displayMin = timeLeft.toString();
								tipTimeWarn.timeout = 60000;
								playSound.loops = 4;
								playSound.play();
								tipTimeWarn.open();
								stop();
								complete = true;
							}
						}
					}
				}
			}
		}

		function init(finalTime) {
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
					else if (contentItem.contentY >= height + navBar.height) {
					//else if (scrollBarPosition - contentItem.contentY <= 70) { //This number is arbitrary, but was chosen after reviewing debugging information
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
				text: qsTr("Trainning day <b>#" + mesoTDay + "</b> of meso cycle <b>" + mesoName +
						"</b>: <b>" + JSF.formatDateToDisplay(mainDate, AppSettings.appLocale) + "</b> Division: <b>" + mesoSplitLetter + "</b>")
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
					//currentIndex: indexOfValue(splitLetter)
					//displayText: cboModel.get(cboSplitLetter.indexOfValue(splitLetter)).text
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

			TPRadioButton {
				id: optFreeTimeSession
				text: qsTr("Open time training session")
				checked: true
				enabled: !timerRestricted.running
				Layout.fillWidth: true

				onClicked: {
					frmOpenTime.visible = true;
					frmConstrainedTime.visible = false;
					optTimeConstrainedSession.checked = false;
				}
			}

			Frame {
				id: frmOpenTime
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
				} //ColumnLayout
			} //Frame

			TPRadioButton {
				id: optTimeConstrainedSession
				text: qsTr("Time constrained session")
				checked: false
				Layout.fillWidth: true

				onClicked: {
					frmConstrainedTime.visible = true;
					frmOpenTime.visible = false;
					optFreeTimeSession.checked = false;
				}
			}

			Frame {
				id: frmConstrainedTime
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 20
				visible: false

				background: Rectangle {
					border.color: "white"
					color: "transparent"
					radius: 6
				}

				ColumnLayout {
					anchors.fill: parent

					RowLayout {
						Layout.fillWidth: true
						Layout.leftMargin: 30

						ButtonFlat {
							id: btnTimeLength
							text: qsTr("By duration")
							enabled: !timerRestricted.running
							Layout.alignment: Qt.AlignCenter
							onClicked: dlgSessionLength.open();
						}
						ButtonFlat {
							id: btnTimeHour
							text: qsTr("By time of day")
							enabled: !timerRestricted.running
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
						Layout.maximumWidth: frmConstrainedTime.width
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

				visible: bHasMesoPlan || bHasPreviousDay
				width: parent.width - 20
				Layout.fillWidth: true
				Layout.bottomMargin: 30
				property int option
				spacing: 0
				padding: 0

				ColumnLayout {
					anchors.fill: parent
					spacing: 0

					TPRadioButton {
						id: optMesoPla
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
									if (grpIntent.option !== 3) {
										removeAllExerciseObjects();
									}
								}
							}

							switch (grpIntent.option) {
								case 0: //use meso plan
									bHasPreviousDay = false;
									loadTrainingDayInfoFromMesoPlan();
								break;
								case 1: //use previous day
									bHasMesoPlan = false;
									loadTrainingDayInfo(previousDivisionDayDate);
									dayId = -1;
								break;
								case 2: //empty session
									bHasPreviousDay = false;
									bHasMesoPlan = false;
								break;
								case 3: //continue session
									bHasPreviousDay = false;
									bHasMesoPlan = false;
								break;
							}
							if (grpIntent.option >=2 )
								placeTipOnAddExercise();
							grpIntent.visible = false;
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

		Component.onCompleted: {
			loadOrCreateDayInfo();
			pageActivation();
		}

		function scrollToPos(y_pos) {
			contentItem.contentY = y_pos;
			if (navButtons === null) {
				var component = Qt.createComponent("PageScrollButtons.qml");
				navButtons = component.createObject(this, {});
				navButtons.scrollTo.connect(setScrollBarPosition);
				navButtons.backButtonWasPressed.connect(maybeShowNavButtons);
			}
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
		if (!loadTrainingDayInfo(mainDate)) {
			checkIfMesoPlanExists();
			checkIfPreviousDayExists();
		}
	}

	function createDatabaseEntryForDay() {
		let result = Database.newTrainingDay(mainDate.getTime(), mesoId, exercisesNames,
			tDay, splitLetter, txtInTime.text, txtOutTime.text, txtLocation.placeholderText, txtDayInfoTrainingNotes.text);
		dayId = result.insertId;
		bModified = false;
	}

	function getMesoPlan () {
		let plan_info = [];
		switch (splitLetter) {
			case 'A': plan_info = Database.getCompleteDivisionAForMeso(mesoId); break;
			case 'B': plan_info = Database.getCompleteDivisionBForMeso(mesoId); break;
			case 'C': plan_info = Database.getCompleteDivisionCForMeso(mesoId); break;
			case 'D': plan_info = Database.getCompleteDivisionDForMeso(mesoId); break;
			case 'E': plan_info = Database.getCompleteDivisionEForMeso(mesoId); break;
			case 'F': plan_info = Database.getCompleteDivisionFForMeso(mesoId); break;
		}
		return plan_info;
	}

	function checkIfMesoPlanExists() {
		let plan_info = getMesoPlan();
		if (plan_info.length > 0) {
			if (plan_info[0].splitExercises)
				bHasMesoPlan = plan_info[0].splitExercises.length > 1;
		}
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

	function loadTrainingDayInfoFromMesoPlan() {
		let plan_info = getMesoPlan();
		exercisesNames = plan_info[0].splitExercises;
		createExercisesFromList();

		const types = plan_info[0].splitSetTypes.split('|');
		const nsets = plan_info[0].splitNSets.split('|');
		const nreps = plan_info[0].splitNReps.split('|');
		const nweights = plan_info[0].splitNWeight.split('|');

		for(var i = 0; i < exerciseSpriteList.length; ++i)
			exerciseSpriteList[i].Object.createSetsFromPlan(nsets[i], types[i], nreps[i], nweights[i]);
		bModified = true;
	}

	function loadTrainingDayInfo(tDate) {
		console.log("Trying to load info for the day: ", tDate.toDateString(), tDate.getTime());
		let dayInfoList = Database.getTrainingDay(tDate.getTime());
		if (dayInfoList.length > 0) {
			if (!dayInfoList[0].exercisesNames) {//Day is saved but it is empty. Treat it as if it weren't saved then
				Database.deleteTraingDay(dayInfoList[0].dayId);
				return false;
			}
			dayId = dayInfoList[0].dayId;
			var tempMesoId = dayInfoList[0].mesoId;
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
			createExercisesFromList();
			return true;
		}
		return false;
	}

	function updateDayIdFromExercisesAndSets() {
		for( var i = 0; i < exerciseSpriteList.length; ++i )
			exerciseSpriteList[i].Object.updateDayId(dayId);
		bModified = true;
	}

	function gotExercise(strName1, strName2, sets, reps, weight, bAdd) {
		if (bAdd) {
			strName1 += ' - ' + strName2;
			addExercise(strName1);
			strName2 = "";
			if (bFirstTime && firstTimeTip)
				firstTimeTip.visible = false;
		}

		var component;
		var exerciseSprite;
		component = Qt.createComponent("ExerciseEntry.qml");

		if (component.status === Component.Ready) {
			var idx = exerciseSpriteList.length;
			exerciseSprite = component.createObject(colExercises, {thisObjectIdx:idx, exerciseName:strName1,
				exerciseName1:strName1, exerciseName2:strName2, tDayId:dayId, stackViewObj:trainingDayPage.StackView.view});
			exerciseSpriteList.push({"Object" : exerciseSprite});
			exerciseSprite.exerciseRemoved.connect(removeExercise);
			exerciseSprite.exerciseEdited.connect(editExercise);
			exerciseSprite.setAdded.connect(addExerciseSet);
			exerciseSprite.requestHideFloatingButtons.connect(hideFloatingButton);

			bStopBounce = true;
			scrollBarPosition = phantomItem.y - trainingDayPage.height + 2*exerciseSprite.height;
			if (scrollBarPosition >= 0)
				scrollTraining.scrollToPos(scrollBarPosition);
			bounceTimer.start();
			if (navButtons !== null)
				navButtons.visible = true;
		}
		else
			console.log("not ready");
	}

	function addExerciseSet(bnewset, exerciseObjIdx, setObject) {
		if (bnewset) {
			bModified = true;
			bStopBounce = true;
			scrollBarPosition = phantomItem.y - trainingDayPage.height + exerciseSpriteList[exerciseObjIdx].Object.height;
			if (exerciseObjIdx === exerciseSpriteList.length-1) {
				scrollBarPosition = phantomItem.y - trainingDayPage.height + exerciseSpriteList[exerciseObjIdx].Object.height + setObject.height;
			}
			else {
				scrollBarPosition = phantomItem.y - trainingDayPage.height + exerciseSpriteList[exerciseObjIdx].Object.height + setObject.y + setObject.height;
			}
			//console.log(exerciseSpriteList[exerciseObjIdx].Object.height);
			//console.log(exerciseObjIdx);
			//console.log(scrollBarPosition);
			scrollTraining.scrollToPos(scrollBarPosition);
			bounceTimer.start();
		}
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

		for( var i = 0, x = 0; i < exerciseSpriteList.length; ++i ) {
			if (i === objidx) {
				removeExerciseName(objidx);
				exerciseSpriteList[objidx].Object.destroy();
			}
			else {
				newObjectList[x] = exerciseSpriteList[i];
				newObjectList[x].Object.thisObjectIdx = x;
				newObjectList[x].Object.updateSetsExerciseIndex();
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

	function createExercisesFromList() {
		const names = exercisesNames.split('|');
		var sep, name, name2 = "";
		for (var i = 0; i < names.length; ++i) {
			sep = names[i].indexOf('&');
			if (sep !== -1) { //Composite exercise
				name = names[i].substring(0, sep);
				name2 = names[i].substring(sep + 1, names[i].length);
			}
			else
				name = names[i];
			gotExercise(name, name2, "0", "0", "0", false);
			exerciseSpriteList[exerciseSpriteList.length-1].Object.bFoldPaneOnLoad = true;
		}
	}

	function removeAllExerciseObjects() {
		if (navButtons !== null)
			navButtons.visible = false;
		const len = exerciseSpriteList.length - 1;
		for (var i = len; i >= 0; --i) {
			exerciseSpriteList[i].Object.destroy();
			exerciseSpriteList.pop();
		}
		exercisesNames = "";
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
		var component;
		var progressSprite;
		component = Qt.createComponent("TrainingDayProgressDialog.qml");

		if (component.status === Component.Ready) {
			progressSprite = component.createObject(trainingDayPage, {calId:calid, splitIdx:splitidx, tDay:day,
							mesoSplit:mesoSplit, mesoEndDate:mesoenddate } );
			progressSprite.init(qsTr("Updating calendar database. Please wait..."), 0, 30);
		}
	}

	function hideFloatingButton(except_idx) {
		for (var x = 0; x < exerciseSpriteList.length; ++x) {
			if (x !== except_idx) {
				if (exerciseSpriteList[x].Object.bFloatButtonVisible)
					exerciseSpriteList[x].Object.bFloatButtonVisible = false;
			}
		}
	}

	function maybeShowNavButtons() {
		navButtons.showButtons();
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
				Database.updateTrainingDay(dayId, exercisesNames, tDay, splitLetter, timeIn, timeOut, location, trainingNotes);

				for (var i = 0; i < exerciseSpriteList.length; ++i)
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
				removeAllExerciseObjects();
				loadOrCreateDayInfo();
				optFreeTimeSession.clicked();
				bModified = false;
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

				var exercise = trainingDayPage.StackView.view.push("ExercisesDatabase.qml", { bChooseButtonEnabled: true });
				exercise.exerciseChosen.connect(gotExercise);
			}
		} // bntAddExercise
	} //footer: ToolBar

	ExercisesListView {
		id: exercisesList
		visible: bShowSimpleExercisesList
		height: mainwindow.height * 0.4
		width: parent.width
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath, multipleSelection) => {
			if (exerciseEntryThatRequestedSimpleList)
				exerciseEntryThatRequestedSimpleList.changeExercise(exerciseName, subName);
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
		var component = Qt.createComponent("FirstTimeHomePageTip.qml");
		if (component.status === Component.Ready) {
			firstTimeTip = component.createObject(trainingDayPage, { message:qsTr("Start here") });
			trainingDayPage.StackView.activating.connect(pageActivation);
			trainingDayPage.StackView.onDeactivating.connect(pageDeActivation);
		}
	}

	function pageActivation() {
		if (bFirstTime) {
			if (grpIntent.visible) {
				scrollTraining.setScrollBarPosition(1);
				grpIntent.highlight = true;
			}
			else
				placeTipOnAddExercise();
		}
	}

	function pageDeActivation() {
		if (firstTimeTip)
			firstTimeTip.visible = false;
	}

	function placeTipOnAddExercise() {
		if (!firstTimeTip)
			createFirstTimeTipComponent();
		firstTimeTip.y = dayInfoToolBar.y;
		firstTimeTip.x = trainingDayPage.width-firstTimeTip.width;
		firstTimeTip.visible = true;
	}
} // Page
