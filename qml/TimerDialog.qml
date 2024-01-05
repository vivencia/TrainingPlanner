import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

import "jsfunctions.js" as JSF

Dialog {
	id: dlgTimer
	closePolicy: simpleTimer ? Popup.CloseOnPressOutside : Popup.NoAutoClose
	modal: false
	width: mainwindow.width - 50
	height: mainwindow.height * 0.35
	x: 25
	y: simpleTimer ? (mainwindow.height / 2) - (height / 2) : 0 // align vertically centered
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0

	property int hours
	property int mins
	property int secs
	property int totalSecs: secs + 60*mins + 3600*hours
	property int origHours
	property int origMins
	property int origSecs
	property bool bJustMinsAndSecs: false
	property bool bJustSecs: false
	property bool simpleTimer
	property bool bRunning: false
	property bool bPaused: false
	property bool bForward: false
	property bool bTimer: true
	property bool bInputOK: true
	property bool bNegCountDown: false
	property bool bTextChanged: false //The user changed the input values and clicked USE before starting the timer(if ever). So we use the values provided by he user
	property string windowTitle

	signal useTime(string time)

	contentItem {
		Keys.onPressed: (event) =>
			processKeyEvents(event);
	}

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
							playSound.play();
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

				if (secs < 59)
					++secs;
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
				onClicked: dlgTimer.close();
			}
		}
	}

	CheckBox {
		id: chkTimer
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.leftMargin: 20
		anchors.topMargin: 0
		height: 40
		text: qsTr("Timer?")
		checked: bTimer
		padding: 0
		spacing: 0

		indicator: Rectangle {
			implicitWidth: 26
			implicitHeight: 26
			x: chkTimer.leftPadding
			y: chkTimer.height / 2 - height / 2
			radius: 5
			border.color: chkTimer.down ? primaryLightColor : paneBackgroundColor
			opacity: 0.5

			Rectangle {
				width: 14
				height: 14
				x: 6
				y: 6
				radius: 2
				color: chkTimer.down ? primaryLightColor : paneBackgroundColor
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
		}
	} // Rectangle recStrings

	Rectangle {
		id: recTimer
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: recStrings.bottom
		height: headerGrid.implicitHeight
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
				radius: 2
				height: 4
				width: 12
				visible: bNegCountDown
				color: "black"
			}
			TextField {
				id: txtHours
				text: JSF.intTimeToStrTime(hours)
				color: enabled ? "black" : "gray"
				enabled: !bJustMinsAndSecs && !bJustSecs
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
					if(activeFocus) {
						txtHours.clear();
					}
					else {
						bInputOK = acceptableInput;
						if (acceptableInput) {
							if (acceptableInput) {
								hours = parseInt(text);
							}
						}
						else
							text = JSF.intTimeToStrTime(hours)
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
			}

			TextField {
				id: txtMinutes
				text: JSF.intTimeToStrTime(mins)
				color: enabled ? "black" : "gray"
				enabled: !bJustSecs
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
					if(activeFocus) {
						txtMinutes.clear();
					}
					else {
						bInputOK = acceptableInput;
						if (acceptableInput) {
							if (acceptableInput) {
								mins = parseInt(text);
							}
						}
						else
							text = JSF.intTimeToStrTime(hours)
					}
				}
				onTextEdited: {
					bInputOK = acceptableInput;
					if (acceptableInput) {
						if ( text.length === 2 ) {
							bTextChanged = true;
							txtSecs.focus = true;
							txtSecs.forceActiveFocus();
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
			}

			TextField {
				id: txtSecs
				text: JSF.intTimeToStrTime(secs)
				color: "black"
				focus: true
				font.pixelSize: AppSettings.fontSizeTitle
				font.bold: true
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
					if(activeFocus) {
						txtSecs.clear();
					}
					else {
						bInputOK = acceptableInput;
						if (acceptableInput) {
							secs = parseInt(text);
						}
						else
							text = JSF.intTimeToStrTime(hours)
					}
				}
				onTextEdited: {
					bInputOK = acceptableInput;
					if (acceptableInput) {
						if ( text.length === 2 ) {
							bTextChanged = true;
							parent.focus = true;
							parent.forceActiveFocus();
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

		Button {
			id: btnStartPause
			text: !bRunning ? qsTr("Start") : qsTr("Pause")
			enabled: bInputOK ? bForward ? true : totalSecs > 0 : false
			font.pixelSize: AppSettings.fontSizePixelSize
			width: 70
			height: 30
			x: ((parent.width / 3) - width ) / 2
			onClicked: {
				playSound.stop();
				if (!bRunning) {
					if (!bPaused) {
						origHours = hours;
						origMins = mins;
						origSecs = secs;
						mainTimer.init();
					}
					else {
						bRunning = true;
						bPaused = false;
						mainTimer.start();
					}
				}
				else {
					bPaused = true;
					mainTimer.stopTimer(false);
				}
			}
		}

		Button {
			id: btnReset
			text: qsTr("Reset")
			font.pixelSize: AppSettings.fontSizePixelSize
			enabled: btnStartPause.enabled
			width: 70
			height: 30
			x: (parent.width / 3) + btnStartPause.x
			onClicked: {
				if (bRunning || bPaused) {
					mainTimer.stopTimer(true);
					playSound.stop();
				}
			}
		}

		Button {
			id: btnUseTime
			text: qsTr("Use")
			font.pixelSize: AppSettings.fontSizePixelSize
			width: 70
			height: 30
			x: btnReset.x + btnReset.width + 2*btnStartPause.x
			onClicked: {
				var totalsecs = secs;
				var totalmins = mins;
				var totalhours = hours;
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
							if (origSecs > secs)
								totalsecs = origSecs - secs;
							else
								totalsecs = secs - origSecs;
							if (origMins > mins)
								totalmins = origMins - mins;
							else
								totalmins = mins - origMins;
							totalhours = origHours - hours;
						}
					}
				}
				if ( totalhours > 0 )
					useTime(JSF.intTimeToStrTime(totalhours) + ":" + JSF.intTimeToStrTime(totalmins) + ":" + JSF.intTimeToStrTime(totalsecs));
				else
					useTime(JSF.intTimeToStrTime(totalmins) + ":" + JSF.intTimeToStrTime(totalsecs));
				if (!simpleTimer) {
					dlgTimer.close();
					mainTimer.stopTimer(true);
				}
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
	}

	function maybeDestroy() {
		if (visible)
			destroy();
	}

	function hideDlg() {
		visible = false;
	}

	function showDlg() {
		visible = true;
	}

	function processKeyEvents(event) {
		if (event.key === Qt.Key_Back) {
			event.accepted = true;
			mainTimer.stopTimer(true);
			dlgTimer.close();
		}
	}
} // Dialog
