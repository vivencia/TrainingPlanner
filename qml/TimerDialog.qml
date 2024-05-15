import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.vivenciasoftware.qmlcomponents

Dialog {
	id: dlgTimer
	closePolicy: simpleTimer ? Popup.CloseOnPressOutside : Popup.NoAutoClose
	modal: false
	width: timePickerOnly ? 150 : windowWidth * 0.75
	height: timePickerOnly ? 100 : windowHeight * 0.35
	x: (windowWidth - width) / 2
	y: simpleTimer ? (windowHeight - height) / 2 - tabMain.height : 0 // align vertically centered
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0

	property bool bJustMinsAndSecs: false
	property bool simpleTimer: false
	property bool timePickerOnly: false
	property bool bInputOK: true
	property bool bNegCountDown: false
	property bool bTextChanged: false //The user changed the input values and clicked USE before starting the timer(if ever). So we use the values provided by he user
	property bool bTempHide: false
	property string windowTitle
	property string initialTime

	readonly property int rowWidth: dlgTimer.width * 0.8
	readonly property int txtWidth: !timePickerOnly ? rowWidth/3 - 20 : 40
	readonly property int leftMarginValue: dlgTimer.width * 0.1

	signal useTime(string time)

	onClosed: mainTimer.stopTimer();

	onInitialTimeChanged: mainTimer.prepareTimer(initialTime);

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
				leftMargin: 10
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
			anchors.rightMargin: 12
			z: 2

			MouseArea {
				anchors.fill: parent
				onClicked: { dlgTimer.close(); }
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

		Row {
			id: recStrings
			height: 30
			spacing: txtWidth
			Layout.minimumWidth: rowWidth
			Layout.maximumWidth: rowWidth
			Layout.leftMargin: !timePickerOnly ? bJustMinsAndSecs ? txtWidth + leftMarginValue : leftMarginValue : 25

			Label {
				color: "darkred"
				font.pointSize: AppSettings.fontSizeLists
				text: qsTr("Hours")
				visible: !bJustMinsAndSecs
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.alignment: Qt.AlignHCenter
			}
			Label {
				color: "darkred"
				font.pointSize: AppSettings.fontSizeLists
				text: qsTr("Minutes")
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.alignment: Qt.AlignHCenter
			}
			Label {
				color: "darkred"
				font.pointSize: AppSettings.fontSizeLists
				text: qsTr("Seconds")
				visible: !timePickerOnly
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.alignment: Qt.AlignHCenter
			}
		} // Rectangle recStrings


		Row {
			id: timerGrid
			spacing: 10
			Layout.topMargin: 5
			Layout.minimumWidth: rowWidth
			Layout.maximumWidth: rowWidth
			Layout.leftMargin: !timePickerOnly ? bJustMinsAndSecs ? txtWidth + leftMarginValue : leftMarginValue : 20

			Rectangle {
				id: recNegCountDown
				radius: 2
				height: 3
				width: 12
				visible: mainTimer.stopWatch !== mainTimer.timerForward
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
					if (activeFocus)
						txtHours.clear();
					else {
						bInputOK = acceptableInput;
						if (acceptableInput)
							mainTimer.strHours = text;
						else
							mainTimer.hours = mainTimer.orignalHours();
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
					if (activeFocus)
						txtMinutes.clear();
					else {
						bInputOK = acceptableInput
						if (acceptableInput)
							mainTimer.strMinutes = text;
						else
							mainTimer.mins = mainTimer.orignalMinutes();
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
					if (activeFocus)
						txtSecs.clear();
					else {
						if (acceptableInput)
							mainTimer.strSeconds = text;
						else
							mainTimer.seconds = mainTimer.orignalSeconds();
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
			width: rowWidth
			from: 0
			to: mainTimer.totalSeconds
			value: mainTimer.seconds
			indeterminate: mainTimer.stopWatch
			Layout.topMargin: 10
			Layout.bottomMargin: 10
			Layout.minimumWidth: width
			Layout.maximumWidth: width
			Layout.leftMargin: leftMarginValue

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

		Row {
			id: btnsRow
			spacing: txtWidth/2
			Layout.topMargin: 3
			Layout.preferredWidth: rowWidth
			Layout.leftMargin: !timePickerOnly ? leftMarginValue : 50
			readonly property int buttonWidth: !timePickerOnly ? rowWidth/3 - 10 : 50

			TPButton {
				id: btnStartPause
				text: mainTimer.active ? qsTr("Pause") : mainTimer.paused ? qsTr("Continue") : qsTr("Start")
				enabled: bInputOK ? mainTimer.stopWatch ? true : mainTimer.totalSeconds > 0 : false
				visible: !timePickerOnly
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

				onClicked: {
					if (!mainTimer.active)
						mainTimer.startTimer();
					else
						mainTimer.stopTimer();
				}
			}

			TPButton {
				id: btnReset
				text: qsTr("Reset")
				visible: !timePickerOnly
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

				onClicked: mainTimer.resetTimer(mainTimer.active);
			}

			TPButton {
				id: btnUseTime
				text: simpleTimer ? qsTr("Close") : timePickerOnly ? qsTr("Done") : qsTr("Use")
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

				onClicked: {
					const elapsedtime = mainTimer.currentElapsedTime();
					if (timePickerOnly)
						useTime(runCmd.formatTime(elapsedtime, false));
					else
						useTime(runCmd.formatTime(elapsedtime, true));
					dlgTimer.close();
				} //btnUseTime
			}
		} //Row
	} //ColumnLayout

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
