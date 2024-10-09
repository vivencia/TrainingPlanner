import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: dlgTimer
	bKeepAbove: !simpleTimer
	width: timePickerOnly ? 150 : appSettings.pageWidth * 0.75
	height: timePickerOnly ? 100 : appSettings.pageHeight * 0.30
	x: (appSettings.pageWidth - width) / 2
	finalYPos: simpleTimer ? (appSettings.pageHeight - height) / 2 : 0 // align vertically centered

	property bool bJustMinsAndSecs: false
	property bool simpleTimer: false
	property bool timePickerOnly: false
	property bool bNegCountDown: false
	property bool bTextChanged: false //The user changed the input values and clicked USE before starting the timer(if ever). So we use the values provided by he user
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
		stopWatch: chkStopWatch.checked

		Component.onCompleted: addWarningAtSecond(15);
	}

	ColumnLayout {
		anchors.fill: parent
		spacing: 0

		Rectangle {
			id: recTitleBar
			height: 20
			width: dlgTimer.width
			color: appSettings.paneBackgroundColor
			z: 0
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8

			Label {
				id: lblTitle
				text: windowTitle
				color: appSettings.fontColor
				height: parent.height
				anchors {
					left: parent.left
					right: parent.right
					rightMargin: 30
					leftMargin: 10
					verticalCenter: parent.verticalCenter
				}
				elide: Text.ElideLeft

				TPMouseArea {
					movableWidget: dlgTimer
					movingWidget: lblTitle
				}
			}

			TPButton {
				id: btnCloseWindow
				imageSource: "close.png"
				hasDropShadow: false
				width: 20
				height: 20

				anchors {
					right: parent.right
					top: parent.top
					rightMargin: 12
				}

				onClicked: { dlgTimer.close(); }
			}
		}

		TPCheckBox {
			id: chkStopWatch
			text: qsTr("Stopwatch")
			checked: false
			visible: !timePickerOnly
			Layout.leftMargin: 10
			Layout.bottomMargin: 10
		} //TPCheckBox

		GridLayout {
			id: recStrings
			height: 30
			columnSpacing: 30
			rowSpacing: 10
			columns: !bJustMinsAndSecs ? 3 : 2
			rows: 2
			Layout.minimumWidth: !bJustMinsAndSecs ? rowWidth : rowWidth * 0.6
			Layout.maximumWidth: !bJustMinsAndSecs ? rowWidth : rowWidth * 0.6
			Layout.leftMargin: !timePickerOnly ? (bJustMinsAndSecs ? txtWidth + leftMarginValue : leftMarginValue) : 25

			Label {
				color: appSettings.fontColor
				font.pointSize: appSettings.fontSizeLists
				text: qsTr("Hours")
				visible: !bJustMinsAndSecs
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.row: 0
				Layout.column: 0
				Layout.leftMargin: 5
			}
			Label {
				color: appSettings.fontColor
				font.pointSize: appSettings.fontSizeLists
				text: qsTr("Minutes")
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.row: 0
				Layout.column: !bJustMinsAndSecs ? 1 : 0
				Layout.leftMargin: -5
			}
			Label {
				color: appSettings.fontColor
				font.pointSize: appSettings.fontSizeLists
				text: qsTr("Seconds")
				visible: !timePickerOnly
				Layout.maximumWidth: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.row: 0
				Layout.column: !bJustMinsAndSecs ? 2 : 1
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
				Layout.row: 1
				Layout.column: 0

				Keys.onPressed: (event) => processKeyEvents(event);

				onActiveFocusChanged: {
					if (activeFocus)
						txtHours.clear();
					else {
						if (acceptableInput)
							mainTimer.strHours = text;
						else
							mainTimer.hours = mainTimer.orignalHours();
					}
				}
				onTextEdited: {
					if (acceptableInput) {
						if ( text.length === 2 ) {
							bTextChanged = true;
							txtMinutes.focus = true;
							txtMinutes.forceActiveFocus();
						}
					}
				}

				Rectangle {
					id: recNegCountDown
					radius: 2
					height: 3
					width: 12
					visible: mainTimer.stopWatch !== mainTimer.timerForward
					color: "black"
					opacity: 0.5

					anchors {
						right: parent.left
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}
				}

				Label {
					text: ":"
					font.pointSize: appSettings.fontSizeText
					font.bold: true
					horizontalAlignment: Text.AlignHCenter
					color: "black"
					width: 10
					visible: txtHours.visible

					anchors {
						left: parent.right
						leftMargin: 10
						verticalCenter: parent.verticalCenter
					}
				}
			} // txtHours

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
				Layout.row: 1
				Layout.column: !bJustMinsAndSecs ? 1 : 0

				Keys.onPressed: (event) => processKeyEvents(event);

				onActiveFocusChanged: {
					if (activeFocus)
						txtMinutes.clear();
					else {
						if (acceptableInput)
							mainTimer.strMinutes = text;
						else
							mainTimer.minutes = mainTimer.orignalMinutes();
					}
				}

				onTextEdited: {
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

				Label {
					text: ":"
					font.pointSize: appSettings.fontSizeText
					font.bold: true
					horizontalAlignment: Text.AlignHCenter
					color: "black"
					width: 10
					visible: txtSecs.visible

					anchors {
						left: parent.right
						leftMargin: 10
						verticalCenter: parent.verticalCenter
					}
				}
			} // txtMinutes

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
				Layout.row: 1
				Layout.column: !bJustMinsAndSecs ? 2 : 1

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
					if (acceptableInput) {
						if ( text.length === 2 ) {
							bTextChanged = true;
							btnStartPause.forceActiveFocus();
						}
					}
				}
			} // txtSecs
		} // GridLayout

		ProgressBar {
			id: progressBar
			visible: !timePickerOnly
			height: 6
			width: rowWidth
			from: mainTimer.totalSeconds
			to: 0
			value: mainTimer.progressValue
			indeterminate: mainTimer.stopWatch
			Layout.topMargin: 5
			Layout.bottomMargin: 5
			Layout.minimumWidth: width
			Layout.maximumWidth: width
			Layout.leftMargin: leftMarginValue

			background: Rectangle {
				implicitWidth: parent.width
				implicitHeight: 6
				color: appSettings.fontColor
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
			Layout.leftMargin: !timePickerOnly ? 10 : 50
			readonly property int buttonWidth: !timePickerOnly ? rowWidth/3 - 10 : 50

			TPButton {
				id: btnStartPause
				text: mainTimer.active ? qsTr("Pause") : mainTimer.paused ? qsTr("Continue") : qsTr("Start")
				flat: false
				enabled: mainTimer.stopWatch ? true : mainTimer.totalSeconds > 0
				visible: !timePickerOnly
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

				onClicked: {
					if (!mainTimer.active)
						mainTimer.startTimer("-");
					else
						mainTimer.stopTimer();
				}
			}

			TPButton {
				id: btnReset
				text: qsTr("Reset")
				flat: false
				visible: !timePickerOnly
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

				onClicked: mainTimer.resetTimer(mainTimer.active);
			}

			TPButton {
				id: btnUseTime
				text: simpleTimer ? qsTr("Close") : timePickerOnly ? qsTr("Done") : qsTr("Use")
				flat: false
				Layout.minimumWidth: btnsRow.buttonWidth
				Layout.maximumWidth: btnsRow.buttonWidth

				onClicked: {
					if (timePickerOnly)
						useTime(mainTimer.strHours + ":" + mainTimer.strMinutes);
					else
						useTime(appUtils.formatTime(mainTimer.currentElapsedTime(), false, true));
					dlgTimer.close();
				} //btnUseTime
			}
		} //Row
	} //ColumnLayout

	function processKeyEvents(event) {
		switch (event.key) {
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
