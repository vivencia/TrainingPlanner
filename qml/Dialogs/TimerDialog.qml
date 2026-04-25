import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
	id: _dlg_timer
	keepAbove: true
	width: AppSettings.pageWidth * 0.75
	height: AppSettings.pageHeight * 0.3
	backgroundRec: timerBackground
	configFieldName: "timerDialogPosition"
	defaultCoordinates: Qt.point((AppSettings.pageWidth - width)/2, (AppSettings.pageHeight - height)/2)

//public:
	property bool timePickerOnly: false
	property bool bNegCountDown: false
	//The user changed the input values and clicked USE before starting the timer(if ever). So we use the values provided by he user
	property bool bTextChanged: false
	property string windowTitle
	property string initialTime

	signal useTime(string time)

//private:
	readonly property int rowWidth: width * 0.8
	readonly property int txtWidth: rowWidth/3 - 20
	readonly property int leftMarginValue: width * 0.05

	onClosed: mainTimer.stopTimer();

	onInitialTimeChanged: mainTimer.prepareTimer(initialTime);

	TPBackRec {
		id: timerBackground
		useImage: true
		rotate_angle: 325
		image_size: Qt.size(width, height)
		border.color: "white"
		sourceImage: ":/images/backgrounds/backimage-timer.png"
		radius: 8
		anchors.fill: parent
		clip: true
	}

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
			enabled: !_dlg_timer.timePickerOnly
			Layout.leftMargin: 10
			Layout.rightMargin: btnClose.width
			Layout.topMargin: -5
			Layout.fillWidth: true

			onClicked: mainTimer.stopWatch = checked;
		}

		RowLayout {
			spacing: 20
			Layout.fillWidth: true
			Layout.leftMargin: _dlg_timer.leftMarginValue

			TPTextInput {
				id: txtHours
				text: mainTimer.strHours
				backgroundOpacity: 0.9
				validator: IntValidator { bottom: 0; top: 99; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				horizontalAlignment: Text.AlignHCenter
				Layout.minimumWidth: _dlg_timer.txtWidth
				Layout.maximumWidth: _dlg_timer.txtWidth
				Layout.alignment: Qt.AlignCenter

				Keys.onPressed: (event) => _dlg_timer.processKeyEvents(event);

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
							_dlg_timer.bTextChanged = true;
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
					font.pixelSize: AppSettings.smallFontSize
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
				font.pixelSize: AppSettings.largeFontSize
				font.bold: true
				horizontalAlignment: Text.AlignHCenter
				visible: txtHours.visible
				Layout.maximumWidth: contentWidth
				Layout.alignment: Qt.AlignCenter
			}

			TPTextInput {
				id: txtMinutes
				text: mainTimer.strMinutes
				backgroundOpacity: 0.9
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				horizontalAlignment: Text.AlignHCenter
				Layout.minimumWidth: _dlg_timer.txtWidth
				Layout.maximumWidth: _dlg_timer.txtWidth
				Layout.alignment: Qt.AlignCenter

				Keys.onPressed: (event) => _dlg_timer.processKeyEvents(event);

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
							_dlg_timer.bTextChanged = true;
							if (!_dlg_timer.timePickerOnly) {
								txtSecs.focus = true;
								txtSecs.forceActiveFocus();
							}
							else
								btnClose.forceActiveFocus();
						}
					}
				}

				TPLabel {
					font.pixelSize: AppSettings.smallFontSize
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
				font.pixelSize: AppSettings.largeFontSize
				font.bold: true
				horizontalAlignment: Text.AlignHCenter
				visible: txtSecs.visible
				Layout.maximumWidth: contentWidth
				Layout.alignment: Qt.AlignCenter
			}

			TPTextInput {
				id: txtSecs
				text: mainTimer.strSeconds
				backgroundOpacity: 0.9
				enabled: !_dlg_timer.timePickerOnly
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				horizontalAlignment: Text.AlignHCenter
				Layout.minimumWidth: _dlg_timer.txtWidth
				Layout.maximumWidth: _dlg_timer.txtWidth
				Layout.alignment: Qt.AlignCenter

				Keys.onPressed: (event) => _dlg_timer.processKeyEvents(event);

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
							_dlg_timer.bTextChanged = true;
							btnStartPause.forceActiveFocus();
						}
					}
				}

				TPLabel {
					font.pixelSize: AppSettings.smallFontSize
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
			enabled: !_dlg_timer.timePickerOnly
			from: mainTimer.totalSeconds
			to: 0
			value: mainTimer.progressValue
			indeterminate: chkStopWatch.checked
			Layout.fillWidth: true
			Layout.preferredWidth: 6
			Layout.rightMargin: _dlg_timer.leftMarginValue
			Layout.leftMargin: _dlg_timer.leftMarginValue

			background: Rectangle {
				implicitWidth: parent.width
				implicitHeight: 6
				color: AppSettings.fontColor
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
			Layout.leftMargin: 5
			Layout.preferredWidth: _dlg_timer.rowWidth

			readonly property int btnWidth: _dlg_timer.width / 3 - 5

			TPButton {
				id: btnStartPause
				text: mainTimer.active ? qsTr("Pause") : mainTimer.paused ? qsTr("Continue") : qsTr("Start")
				enabled: !_dlg_timer.timePickerOnly ? mainTimer.stopWatch ? true : mainTimer.totalSeconds > 0 : false
				Layout.preferredWidth: btnsRow.btnWidth

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
				enabled: !_dlg_timer.timePickerOnly
				Layout.preferredWidth: btnsRow.btnWidth

				onClicked: mainTimer.resetTimer(mainTimer.active);
			}

			TPButton {
				id: btnClose
				text: _dlg_timer.timePickerOnly ? qsTr("Done") : qsTr("Close")
				Layout.preferredWidth: btnsRow.btnWidth

				onClicked: {
					if (_dlg_timer.timePickerOnly)
						_dlg_timer.useTime(mainTimer.strHours + ":" + mainTimer.strMinutes);
					_dlg_timer.closePopup();
				} //btnClose
			}
		} //Row
	} //ColumnLayout

	function processKeyEvents(event) {
		switch (event.key) {
			case Qt.Key_Enter:
			case Qt.Key_Return:
			if (!_dlg_timer.timePickerOnly) {
				if (btnStartPause.enabled) {
					btnStartPause.forceActiveFocus();
					btnStartPause.clicked();
				}
			}
			else {
				bTextChanged = true;
				_dlg_timer.btnClose.forceActiveFocus();
				_dlg_timer.btnClose.clicked();
			}
			break;
		}
	}
} // Dialog
