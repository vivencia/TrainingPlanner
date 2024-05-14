import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.vivenciasoftware.qmlcomponents

Dialog {
	id: dlgTimer
	closePolicy: simpleTimer ? Popup.CloseOnPressOutside : Popup.NoAutoClose
	modal: false
	width: timePickerOnly ? 150 : simpleTimer ? windowWidth * 0.75 : windowWidth
	height: timePickerOnly ? 100 : windowHeight * 0.35
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
	property bool bInputOK: true
	property bool bNegCountDown: false
	property bool bTextChanged: false //The user changed the input values and clicked USE before starting the timer(if ever). So we use the values provided by he user
	property bool bTempHide: false
	property bool bWasSuspended: false
	property date suspendedTime
	property date suspendedTimer
	property string windowTitle

	readonly property int txtWidth: !timePickerOnly ? (dlgTimer.width * 0.8)/3 - 20 : 40

	signal useTime(string time)

	TPTimer {
		id: mainTimer
		interval: 1000
		alarmSoundFile: "qrc:/sounds/timer-end.wav"
		stopWatch: !chkTimer.checked

		Component.onCompleted: {
			setRunCommandsObject(runCmd);
			addWarningAtSecond(15);
		}
	}

	header: Rectangle {
		id: recTitleBar
		height: 20
		width: dlgTimer.width
		color: AppSettings.paneBackgroundColor
		opacity: 0.5
		z: 0

		Label {
			id: lblTitle
			text: windowTitle
			color: AppSettings.fontColor
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
			source: "qrc:/images/"+AppSettings.iconFolder+"close.png"
			width: parent.height
			height: parent.height
			anchors.right: parent.right
			anchors.top: parent.top
			anchors.rightMargin: 10
			z: 2

			MouseArea {
				anchors.fill: parent
				onClicked: { mainTimer.stopTimer(); dlgTimer.close(); }
			}
		}
	}

	ColumnLayout {
		x: 0
		y: 0
		spacing: 0

		TPCheckBox {
			id: chkTimer
			text: qsTr("Timer?")
			textColor: "darkred"
			checked: true
			visible: !timePickerOnly
			Layout.leftMargin: 10
		} //TPCheckBox

		RowLayout {
			id: recStrings
			height: 30
			Layout.preferredWidth: dlgTimer.width * 0.8
			Layout.leftMargin: !timePickerOnly ? dlgTimer.width * 0.1 : 25

			Label {
				color: "darkred"
				font.pointSize: AppSettings.fontSizeLists
				text: qsTr("Hours")
				visible: !bJustMinsAndSecs
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
			}
			Label {
				color: "darkred"
				font.pointSize: AppSettings.fontSizeLists
				text: qsTr("Minutes")
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
			}
			Label {
				color: "darkred"
				font.pointSize: AppSettings.fontSizeLists
				text: qsTr("Seconds")
				visible: !timePickerOnly
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
			}
		} // Rectangle recStrings


		RowLayout {
			id: timerGrid
			spacing: 0
			Layout.preferredWidth: dlgTimer.width * 0.8
			Layout.leftMargin: !timePickerOnly ? dlgTimer.width * 0.1 : 20

			Rectangle {
				id: recNegCountDown
				radius: 2
				height: 3
				width: 12
				visible: bNegCountDown
				color: "black"
				opacity: 0.5
				Layout.alignment: Qt.AlignVCenter
				Layout.leftMargin: -10
			}

			TPTextInput {
				id: txtHours
				text: mainTimer.strHours
				visible: !bJustMinsAndSecs
				focus: true
				validator: IntValidator { bottom: 0; top: 99; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				width: txtWidth
				horizontalAlignment: Text.AlignHCenter
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth

				Keys.onPressed: (event) => processKeyEvents(event);

				onActiveFocusChanged: {
					if (activeFocus) {
						txtHours.clear();
						origHours = mainTimer.hours;
					}
					else {
						bInputOK = acceptableInput;
						if (acceptableInput)
							mainTimer.strHours = text;
						else
							mainTimer.hours = origHours;
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
				text: ":  "
				font.pointSize: AppSettings.fontSizeText
				fontSizeMode: Text.Fit
				font.bold: true
				color: "black"
				visible: txtHours.visible
				Layout.maximumWidth: 10
				Layout.minimumWidth: 10
			}

			TPTextInput {
				id: txtMinutes
				text: mainTimer.strMinutes
				focus: true
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				width: txtWidth
				horizontalAlignment: Text.AlignHCenter
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth

				Keys.onPressed: (event) => processKeyEvents(event);

				onActiveFocusChanged: {
					if (activeFocus) {
						txtMinutes.clear();
						origMins = mainTimer.minutes;
					}
					else {
						bInputOK = acceptableInput
						if (acceptableInput)
							mainTimer.strMinutes = text;
						else
							mainTimer.mins = origMins;
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
				text: ":  "
				font.pointSize: AppSettings.fontSizeText
				fontSizeMode: Text.Fit
				font.bold: true
				color: "black"
				visible: txtSecs.visible
				Layout.maximumWidth: 10
				Layout.minimumWidth: 10
			}

			TPTextInput {
				id: txtSecs
				text: mainTimer.strSeconds
				focus: true
				visible: !timePickerOnly
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				width: txtWidth
				horizontalAlignment: Text.AlignHCenter
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth

				Keys.onPressed: (event) => processKeyEvents(event);

				onActiveFocusChanged: {
					if (activeFocus) {
						txtSecs.clear();
						origSecs = mainTimer.seconds;
					}
					else {
						if (acceptableInput)
							mainTimer.strSeconds = text;
						else
							mainTimer.seconds = origSecs;
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

		ProgressBar {
			id: progressBar
			visible: !timePickerOnly
			height: 6
			width: dlgTimer.width*0.8
			from: 0
			to: mainTimer.totalSecs
			value: mainTimer.seconds
			indeterminate: mainTimer.stopWatch
			Layout.topMargin: 10
			Layout.bottomMargin: 10
			Layout.minimumWidth: width
			Layout.maximumWidth: width
			Layout.leftMargin: dlgTimer.width * 0.1

			background: Rectangle {
				implicitWidth: parent.width
				implicitHeight: 6
				color: "black"
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

		RowLayout {
			id: btnsRow
			spacing: 0
			Layout.topMargin: 3
			Layout.preferredWidth: dlgTimer.width * 0.8
			Layout.leftMargin: !timePickerOnly ? dlgTimer.width * 0.1 : 50
			readonly property int buttonWidth: !timePickerOnly ? (dlgTimer.width * 0.8)/3 - 10 : 50

			TPButton {
				id: btnStartPause
				text: qsTr("Start")
				enabled: bInputOK ? mainTimer.stopWatch ? true : mainTimer.totalSecs() > 0 : false
				visible: !timePickerOnly
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

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

			TPButton {
				id: btnReset
				text: qsTr("Reset")
				visible: !timePickerOnly
				enabled: bRunning ? false : bPaused
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

				onClicked: {
					mainTimer.stopTimer(true);
					playSound.stop();
					btnStartPause.text = qsTr("Start");
				}
			}

			TPButton {
				id: btnUseTime
				text: simpleTimer ? qsTr("Close") : timePickerOnly ? qsTr("Done") : qsTr("Use")
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

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
				} //btnUseTime
			}
		} //Row
	} //ColumnLayout

	onClosed: playSound.stop();

	Component.onCompleted: {
		mainwindow.backButtonPressed.connect(maybeDestroy);
		mainwindow.mainMenuOpened.connect(hideDlg);
		mainwindow.mainMenuClosed.connect(showDlg);
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
} // Dialog
