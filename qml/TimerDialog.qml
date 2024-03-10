import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Dialog {
	id: dlgTimer
	closePolicy: simpleTimer ? Popup.CloseOnPressOutside : Popup.NoAutoClose
	modal: false
	width: simpleTimer ? windowWidth * 0.75 : windowWidth
	height: windowHeight * 0.30
	x: (windowWidth - width) / 2
	y: simpleTimer ? (windowHeight - height) / 2 - tabMain.height : 0 // align vertically centered
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0

	property int hours: 0
	property int mins: 0
	property int secs: 0
	property int totalSecs: secs + 60*mins + 3600*hours
	property int origHours: 0
	property int origMins: 0
	property int origSecs: 0
	property bool bJustMinsAndSecs: false
	property bool simpleTimer: false
	property bool timePickerOnly: false
	property bool bRunning: false
	property bool bPaused: false
	property bool bForward: false
	property bool bTimer: true
	property bool bInputOK: true
	property bool bNegCountDown: false
	property bool bTextChanged: false //The user changed the input values and clicked USE before starting the timer(if ever). So we use the values provided by he user
	property bool bTempHide: false
	property bool bWasSuspended: false
	property date suspendedTime
	property int suspendedHours;
	property int suspendedMins;
	property int suspendedSecs;
	property string windowTitle

	signal useTime(string time)

	SoundEffect {
		id: playSound
		source: "qrc:/sounds/timer-end.wav"
		loops: SoundEffect.Infinite
	}

	Timer {
		id: mainTimer
		interval: 1000
		running: false
		repeat: true
		property int indVal: 1

		onTriggered: {
			if (!bForward) {
				progressBar.value = progressBar.value - 1;
				if (totalSecs <= 14 && !playSound.playing)
					playSound.play();
				if (secs > 0) {
					--secs;
				}
				else if (secs === 0) {
					if (mins > 0 ) {
						secs = 59;
						--mins;
					}
					else if ( mins === 0 ) {
						if (hours > 0) {
							--hours;
							if (mins === 0) {
								mins = 59;
								secs = 59;
							}
							else
								--mins;
						}
						else {
							//playSound.play();
							bForward = true;
							progressBar.indeterminate = true;
							progressBar.value = 0;
							progressBar.from = 0;
							progressBar.to = 10;
							bNegCountDown = true;
						}
					}
				}
			}
			else {
				switch (progressBar.value) {
					case 0: indVal = 1; break;
					case 10: indVal = -1; break;
					default: break;
				}
				progressBar.value = progressBar.value + indVal;

				if (secs < 59) {
					++secs;
					if (secs >= 4 && playSound.playing)
						playSound.stop();
				}
				else {
					if (mins < 59) {
						secs = 0;
						++mins;
					}
					else {
						if (hours < 99) {
							mins = 0;
							++hours
						}
						else
							stopTimer(!bForward); //When in stopwath mode, leave the current values displayed
					}
				}
			}
		}

		function init () {
			//const totalSecs = secs + 60*mins + 3600*hours
			if (bForward) {
				progressBar.indeterminate = true;
				progressBar.value = 0;
				progressBar.from = 0;
				progressBar.to = 10;
			}
			else {
				progressBar.indeterminate = false;
				progressBar.from = totalSecs;
				progressBar.to = 0;
				progressBar.value = totalSecs;
			}
			bRunning = true;
			bPaused = false;
			bTextChanged = false;
			mainTimer.start();
		}

		function stopTimer(reset) {
			stop();
			bRunning = false;
			if (reset) {
				if (bNegCountDown) {
					bForward = false;
					bNegCountDown = false;
				}
				hours = origHours;
				mins = origMins;
				secs = origSecs;
				progressBar.value = 0;
				bPaused = false;
			}
		}
	}

	header: Rectangle {
		id: recTitleBar
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			leftMargin: 0
			topMargin: 0
			rightMargin: 0
			bottomMargin: 0
		}
		height: 20
		width: dlgTimer.width
		color: paneBackgroundColor
		opacity: 0.5
		z: 0

		Label {
			id: lblTitle
			text: windowTitle
			color: "black"
			height: parent.height
			anchors {
				left: parent.left
				right: parent.right
				rightMargin: 30
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}
			elide: Text.ElideLeft
		}

		MouseArea {
			id: titleBarMouseRegion
			property var prevPos
			anchors.fill: parent
			z: 1
			onPressed: (mouse) => {
				prevPos = { x: mouse.x, y: mouse.y };
			}
			onPositionChanged: {
				const deltaX = mouseX - prevPos.x;
				if (Math.abs(deltaX) < 10) {
					const deltaY = mouseY - prevPos.y;
					if (Math.abs(deltaY) < 10) {
						dlgTimer.x += deltaX;
						dlgTimer.y += deltaY;
					}
				}
				prevPos = { x: mouseX, y: mouseY };
			}
		}

		Image {
			id: btnCloseWindow
			source: "qrc:/images/"+darkIconFolder+"close.png"
			width: parent.height
			height: parent.height
			anchors.right: parent.right
			anchors.top: parent.top
			anchors.rightMargin: 10
			z: 2

			MouseArea {
				anchors.fill: parent
				onClicked: { mainTimer.stopTimer(reset); dlgTimer.close(); }
			}
		}
	}

	CheckBox {
		id: chkTimer
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.leftMargin: 5
		anchors.topMargin: -20
		text: qsTr("Timer?")
		checked: bTimer
		padding: 0
		spacing: 0
		visible: !timePickerOnly

		indicator: Rectangle {
			implicitWidth: 26
			implicitHeight: 26
			x: chkTimer.leftPadding
			y: chkTimer.height / 2 - height / 2
			radius: 5
			border.color: chkTimer.down ? primaryDarkColor : primaryLightColor
			opacity: 0.5

			Rectangle {
				width: 14
				height: 14
				x: 6
				y: 6
				radius: 2
				color: chkTimer.down ? primaryDarkColor : primaryLightColor
				visible: chkTimer.checked
				opacity: 0.5
			}
		}

		contentItem: Text {
			text: chkTimer.text
			wrapMode: Text.WordWrap
			opacity: enabled ? 1.0 : 0.3
			verticalAlignment: Text.AlignVCenter
			leftPadding: chkTimer.indicator.width + chkTimer.spacing
			color: "darkred"
		}

		onToggled: {
			bTimer = checked;
			bForward = !checked;
			if (bForward && totalSecs === 0)
				bInputOK = true;
		}
	} // CheckBox

	Rectangle {
		id: recStrings
		anchors.top: chkTimer.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.topMargin: 0
		height: 30
		width: recTimer.width

		Label {
			x: 0
			y: 10
			color: "darkred"
			font.pixelSize: AppSettings.fontSizeLists
			text: qsTr("Hours")
			visible: !bJustMinsAndSecs
		}
		Label {
			y: 10
			x: txtMinutes.x - 10
			color: "darkred"
			font.pixelSize: AppSettings.fontSizeLists
			text: qsTr("Minutes")
		}
		Label {
			y: 10
			x: txtSecs.x - 10
			color: "darkred"
			font.pixelSize: AppSettings.fontSizeLists
			text: qsTr("Seconds")
			visible: !timePickerOnly
		}
	} // Rectangle recStrings

	Rectangle {
		id: recTimer
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: recStrings.bottom
		height: headerGrid.implicitHeight - 10
		width: headerGrid.implicitWidth + 20
		color: paneBackgroundColor
		opacity: 0.5
		radius: 5

		RowLayout {
			id: headerGrid
			anchors.fill: parent

			Rectangle {
				id: recNegCountDown
				Layout.alignment: Qt.AlignVCenter
				Layout.topMargin: -height
				radius: 2
				height: 4
				width: 12
				visible: bNegCountDown
				color: "black"
			}
			TextField {
				id: txtHours
				text: runCmd.intTimeToStrTime(hours)
				color: enabled ? "black" : "gray"
				visible: !bJustMinsAndSecs
				focus: true
				font.pixelSize: AppSettings.fontSizeTitle
				font.bold: true
				validator: IntValidator { bottom: 0; top: 99; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				Layout.maximumWidth: 30
				Layout.alignment: Text.AlignHCenter
				Layout.leftMargin: 10

				Keys.onPressed: (event) => processKeyEvents(event);

				background: Rectangle {
					color: "transparent"
				}

				onActiveFocusChanged: {
					if (activeFocus) {
						txtHours.clear();
						placeholderText = runCmd.intTimeToStrTime(hours);
						origHours = hours;
					}
					else {
						bInputOK = acceptableInput;
						if (acceptableInput)
							hours = parseInt(text);
						else
							hours = origHours;
					}
				}
				onTextEdited: {
					bInputOK = acceptableInput;
					if (acceptableInput) {
						if ( text.length === 2 ) {
							bTextChanged = true;
							txtMinutes.focus = true;
							txtMinutes.forceActiveFocus();
						}
					}
				}
			} // txtHours

			Label {
				text: "  :  "
				font.pixelSize: AppSettings.fontSizeText
				fontSizeMode: Text.Fit
				font.bold: true
				opacity: 0.6
				color: "black"
				visible: txtHours.visible
			}

			TextField {
				id: txtMinutes
				text: runCmd.intTimeToStrTime(mins)
				color: enabled ? "black" : "gray"
				Layout.alignment: Text.AlignHCenter
				focus: true
				font.pixelSize: AppSettings.fontSizeTitle
				font.bold: true
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				Layout.maximumWidth: 30

				Keys.onPressed: (event) => processKeyEvents(event);

				background: Rectangle {
					color: "transparent"
				}

				onActiveFocusChanged: {
					if (activeFocus) {
						txtMinutes.clear();
						placeholderText = runCmd.intTimeToStrTime(mins);
						origMins = mins;
					}
					else {
						bInputOK = acceptableInput
						if (acceptableInput)
							mins = parseInt(text);
						else
							mins = origMins;
					}
				}
				onTextEdited: {
					bInputOK = acceptableInput;
					if (acceptableInput) {
						if (text.length === 2) {
							bTextChanged = true;
							if (!timePickerOnly) {
								txtSecs.focus = true;
								txtSecs.forceActiveFocus();
							}
							else
								btnUseTime.forceActiveFocus();
						}
					}
				}
			} // txtMinutes

			Label {
				text: "  :  "
				font.pixelSize: AppSettings.fontSizeText
				fontSizeMode: Text.Fit
				font.bold: true
				opacity: 0.6
				color: "black"
				visible: txtSecs.visible
			}

			TextField {
				id: txtSecs
				text: runCmd.intTimeToStrTime(secs)
				color: "black"
				focus: true
				font.pixelSize: AppSettings.fontSizeTitle
				font.bold: true
				visible: !timePickerOnly
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				Layout.alignment: Text.AlignHCenter
				Layout.maximumWidth: 30
				Layout.rightMargin: 10

				Keys.onPressed: (event) => processKeyEvents(event);

				background: Rectangle {
					color: "transparent"
				}

				onActiveFocusChanged: {
					if (activeFocus) {
						txtSecs.clear();
						placeholderText = runCmd.intTimeToStrTime(secs);
						origSecs = secs;
					}
					else {
						if (acceptableInput)
							secs = parseInt(text);
						else
							secs = origSecs;
					}
				}
				onTextEdited: {
					bInputOK = acceptableInput;
					if (acceptableInput) {
						if ( text.length === 2 ) {
							bTextChanged = true;
							btnStartPause.forceActiveFocus();
						}
					}
				}
			} // txtSecs
		} // headerGrid
	} // Rectangle recTimer

	Rectangle {
		id: recProgress
		anchors.top: recTimer.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		height: 20
		width: recTimer.width
		visible: !timePickerOnly

		ProgressBar {
			id: progressBar
			width: parent.width
			y: (parent.height / 2) - 3

			background: Rectangle {
				implicitWidth: parent.width
				implicitHeight: 6
				color: paneBackgroundColor
				opacity: 0.5
				radius: 3
			}

			contentItem: Item {
				implicitWidth: parent.width
				implicitHeight: 6
				Rectangle {
					width: progressBar.visualPosition * parent.width
					height: parent.height
					radius: 2
				}
			}
		} // ProgressBar
	} // Rectangle

	Rectangle {
		id: recButtons
		anchors.top: recProgress.bottom
		anchors.left: parent.left
		height: 40
		width: parent.width

		ButtonFlat {
			id: btnStartPause
			enabled: bInputOK ? bForward ? true : totalSecs > 0 : false
			visible: !timePickerOnly
			width: 70
			height: 30
			text: qsTr("Start")
			x: ((parent.width / 3) - width ) / 2

			onClicked: {
				playSound.stop();
				if (!bRunning) {
					if (!bPaused) { //Start
						origHours = hours;
						origMins = mins;
						origSecs = secs;
						mainTimer.init();
					}
					else { //Continue
						bPaused = false;
						bRunning = true;
						mainTimer.start();
					}
					text = qsTr("Pause");
				}
				else { //Pause
					if (!bPaused) {
						bPaused = true;
						mainTimer.stopTimer(false);
						text = qsTr("Continue");
					}
				}
			}
		}

		ButtonFlat {
			id: btnReset
			text: qsTr("Reset")
			visible: !timePickerOnly
			font.pixelSize: AppSettings.fontSizePixelSize
			enabled: bRunning ? false : bPaused
			width: 70
			height: 30
			x: (parent.width / 3) + btnStartPause.x
			onClicked: {
				mainTimer.stopTimer(true);
				playSound.stop();
				btnStartPause.text = qsTr("Start");
			}
		}

		ButtonFlat {
			id: btnUseTime
			text: simpleTimer ? qsTr("Close") : timePickerOnly ? qsTr("Done") : qsTr("Use")
			font.pixelSize: AppSettings.fontSizePixelSize
			width: 70
			height: 30
			x: btnReset.x + btnReset.width + 2*btnStartPause.x

			onClicked: {
				var totalsecs = secs;
				var totalmins = mins;
				var totalhours = hours;
				if (!simpleTimer) {
					if (!bTextChanged) {
						if (bTimer) {
							if (bForward) { // Elapsed time after zero plus the starting time
								totalsecs += origSecs;
								totalmins += origMins;
								totalhours += origHours;
								if (totalsecs > 59) {
									totalsecs -= 59;
									totalmins++;
								}
								if (totalmins > 59) {
									totalmins -= 59;
									totalhours++;
								}
								if (totalhours > 99)
									totalhours = 99;
							}
							else { //Compute elapsed time
								const elapsedTime = origSecs + 60*origMins + 3600*origHours - totalSecs;
								totalhours = Math.floor(elapsedTime / 3600);
								totalmins = elapsedTime - totalhours * 3600;
								totalmins = Math.floor(totalmins / 60);
								totalsecs = elapsedTime - totalhours * 3600 - totalmins * 60;
							}
						}
					}
					if (timePickerOnly) {
						useTime(runCmd.intTimeToStrTime(hours) + ":" + runCmd.intTimeToStrTime(mins));
					}
					else {
						if ( totalhours > 0 )
							useTime(runCmd.intTimeToStrTime(totalhours) + ":" + runCmd.intTimeToStrTime(totalmins) + ":" + runCmd.intTimeToStrTime(totalsecs));
						else
							useTime(runCmd.intTimeToStrTime(totalmins) + ":" + runCmd.intTimeToStrTime(totalsecs));
					}
				}
				dlgTimer.close();
				mainTimer.stopTimer(true);
				btnStartPause.text = qsTr("Start");
			}
		}
	} // recButtons

	onClosed: {
		playSound.stop();
	}

	Component.onCompleted: {
		mainwindow.backButtonPressed.connect(maybeDestroy);
		mainwindow.mainMenuOpened.connect(hideDlg);
		mainwindow.mainMenuClosed.connect(showDlg);
		if (Qt.platform.os === "android") {
			mainwindow.appAboutToBeSuspended.connect(appSuspended);
			mainwindow.appActive.connect(appActivated);
		}
	}

	function maybeDestroy() {
		if (visible)
			destroy();
	}

	function hideDlg() {
		if (visible) {
			bTempHide = true;
			visible = false;
		}
	}

	function showDlg() {
		if (bTempHide) {
			visible = true;
			bTempHide = false;
		}
	}

	function processKeyEvents(event) {
		switch (event.key) {
			case Qt.Key_Back:
				event.accepted = true;
				mainTimer.stopTimer(true);
				dlgTimer.close();
			break;
			case Qt.Key_Enter:
			case Qt.Key_Return:
				if (!timePickerOnly) {
					if (btnStartPause.enabled) {
						btnStartPause.forceActiveFocus();
						btnStartPause.clicked();
					}
				}
				else {
					bTextChanged = true;
					btnUseTime.forceActiveFocus();
					btnUseTime.clicked();
				}

			break;
		}
	}

	function appSuspended() {
		if (bRunning) {
			bWasSuspended = true;
			suspendedTime = new Date();
			suspendedHours = hours;
			suspendedMins = mins;
			suspendedSecs = secs;
		}
	}

	function appActivated() {
		if (bWasSuspended) {
			bWasSuspended = false;
			const now = new Date();
			var correctedHours = 0, correctedMins = 0, correctedSecs = 0;
			if (bTimer) {
				correctedHours = suspendedHours - (now.getHours() - suspendedTime.getHours());
				correctedMins = suspendedMins - (now.getMinutes() - suspendedTime.getMinutes());
				correctedSecs = suspendedSecs - (now.getSeconds() - suspendedTime.getSeconds());
				if (correctedSecs < 0) {
					correctedSecs += 60;
					correctedMins--;
				}
				if (correctedMins < 0) {
					correctedMins += 60;
					correctedHours--;
				}
				if (correctedHours < 0)
					correctedHours = 0;
			}
			else {
				correctedHours = suspendedHours + (now.getHours() - suspendedTime.getHours());
				correctedMins = suspendedMins + (now.getMinutes() - suspendedTime.getMinutes());
				correctedSecs = suspendedSecs + (now.getSeconds() - suspendedTime.getSeconds());
				if (correctedSecs > 59) {
					correctedSecs -= 59;
					correctedMins++;
				}
				if (correctedMins > 59) {
					correctedMins -= 59;
					++correctedHours;
				}
			}
			//console.log("correctedHours:  " + correctedHours + "    hours:   " + hours);
			//console.log("correctedMins:  " + correctedMins + "   mins:   ", mins);
			//console.log("correctedSecs:  " + correctedSecs + "   secs:   ", secs);
			hours = correctedHours;
			mins = correctedMins;
			secs = correctedSecs;
		}
	}
} // Dialog
