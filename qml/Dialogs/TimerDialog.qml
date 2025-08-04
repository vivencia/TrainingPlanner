import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: dlgTimer
	keepAbove: true
	width: appSettings.pageWidth * 0.75
	height: appSettings.pageHeight * 0.3
	x: (appSettings.pageWidth - width) / 2
	finalYPos: (appSettings.pageHeight - height) / 2

	property bool timePickerOnly: false
	property bool bNegCountDown: false
	property bool bTextChanged: false //The user changed the input values and clicked USE before starting the timer(if ever). So we use the values provided by he user
	property string windowTitle
	property string initialTime

	readonly property int rowWidth: width * 0.8
	readonly property int txtWidth: rowWidth/3 - 20
	readonly property int leftMarginValue: width * 0.05

	signal useTime(string time)

	onClosed: mainTimer.stopTimer();

	onInitialTimeChanged: mainTimer.prepareTimer(initialTime);

	TPTimer {
		id: mainTimer
		interval: 1000
		alarmSoundFile: "qrc:/sounds/timer-end.wav"
		stopWatch: false

		Component.onCompleted: addWarningAtSecond(15);
	}

	ColumnLayout {
		anchors.fill: parent
		spacing: 0

		TPRadioButtonOrCheckBox {
			id: chkStopWatch
			text: qsTr("Stopwatch")
			radio: false
			checked: false
			enabled: !timePickerOnly
			Layout.leftMargin: 10
			Layout.rightMargin: btnClose.width
			Layout.fillWidth: true

			onClicked: mainTimer.stopWatch = checked;
		}

		RowLayout {
			spacing: 20
			Layout.fillWidth: true
			Layout.leftMargin: leftMarginValue

			TPTextInput {
				id: txtHours
				text: mainTimer.strHours
				focus: true
				validator: IntValidator { bottom: 0; top: 99; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				horizontalAlignment: Text.AlignHCenter
				width: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.maximumWidth: txtWidth
				Layout.alignment: Qt.AlignCenter

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

				TPLabel {
					font.pixelSize: appSettings.smallFontSize
					text: qsTr("Hours")

					anchors {
						horizontalCenter: parent.horizontalCenter
						bottom: parent.top
						bottomMargin: 10
					}
				}
			} // txtHours

			TPLabel {
				text: ":"
				font.pixelSize: appSettings.largeFontSize
				font.bold: true
				horizontalAlignment: Text.AlignHCenter
				visible: txtHours.visible
				Layout.maximumWidth: contentWidth
				Layout.alignment: Qt.AlignCenter
			}

			TPTextInput {
				id: txtMinutes
				text: mainTimer.strMinutes
				focus: true
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				horizontalAlignment: Text.AlignHCenter
				width: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.maximumWidth: txtWidth
				Layout.alignment: Qt.AlignCenter

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
								btnClose.forceActiveFocus();
						}
					}
				}

				TPLabel {
					font.pixelSize: appSettings.smallFontSize
					text: qsTr("Minutes")

					anchors {
						horizontalCenter: parent.horizontalCenter
						bottom: parent.top
						bottomMargin: 10
					}
				}
			} // txtMinutes

			TPLabel {
				text: ":"
				font.pixelSize: appSettings.largeFontSize
				font.bold: true
				horizontalAlignment: Text.AlignHCenter
				visible: txtSecs.visible
				Layout.maximumWidth: contentWidth
				Layout.alignment: Qt.AlignCenter
			}

			TPTextInput {
				id: txtSecs
				text: mainTimer.strSeconds
				focus: true
				enabled: !timePickerOnly
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				horizontalAlignment: Text.AlignHCenter
				width: txtWidth
				Layout.minimumWidth: txtWidth
				Layout.maximumWidth: txtWidth
				Layout.alignment: Qt.AlignCenter

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

				TPLabel {
					font.pixelSize: appSettings.smallFontSize
					text: qsTr("Seconds")
					enabled: parent.enabled

					anchors {
						horizontalCenter: parent.horizontalCenter
						bottom: parent.top
						bottomMargin: 10
					}
				}
			} // txtSecs
		} // GridLayout

		ProgressBar {
			id: progressBar
			enabled: !timePickerOnly
			height: 6
			from: mainTimer.totalSeconds
			to: 0
			value: mainTimer.progressValue
			indeterminate: chkStopWatch.checked
			Layout.fillWidth: true
			Layout.rightMargin: leftMarginValue
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

		RowLayout {
			id: btnsRow
			spacing: 5
			Layout.preferredWidth: rowWidth
			Layout.leftMargin: 10

			TPButton {
				id: btnStartPause
				text: mainTimer.active ? qsTr("Pause") : mainTimer.paused ? qsTr("Continue") : qsTr("Start")
				autoSize: true
				flat: false
				enabled: !timePickerOnly ? mainTimer.stopWatch ? true : mainTimer.totalSeconds > 0 : false
				Layout.maximumWidth: dlgTimer.width / 3

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
				autoSize: true
				flat: false
				enabled: !timePickerOnly
				Layout.maximumWidth: dlgTimer.width / 3

				onClicked: mainTimer.resetTimer(mainTimer.active);
			}

			TPButton {
				id: btnClose
				text: timePickerOnly ? qsTr("Done") : qsTr("Close")
				autoSize: true
				flat: false
				Layout.maximumWidth: dlgTimer.width / 3

				onClicked: {
					if (timePickerOnly)
						useTime(mainTimer.strHours + ":" + mainTimer.strMinutes);
					dlgTimer.closePopup();
				} //btnClose
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
					btnClose.forceActiveFocus();
					btnClose.clicked();
				}
			break;
		}
	}
} // Dialog
