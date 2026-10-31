import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Dialogs
import TpQml.Widgets

TPPopup {
	id: _dialog
	objectName: "timerDialog"
	keepAbove: true
	backGroundImage: ":/images/backgrounds/backimage-timer.png"
	backgroundRotation: 325
	showBorder: true
	showTitleBar: true
	savePopupState: true
	show_position: Qt.AlignBaseline
	normal_size: Qt.size(AppSettings.pageWidth * 0.75, AppSettings.pageHeight * 0.3)
	defaultCoordinates: Qt.point((AppSettings.pageWidth - width)/2, (AppSettings.pageHeight - height)/2)

//public:
	property bool timePickerOnly: false
	property bool bNegCountDown: false
	property string windowTitle
	property string initialTime

	signal useTime(string time)

//private:
	readonly property int rowWidth: width * 0.8
	readonly property int txtWidth: rowWidth/3 - 20
	readonly property int leftMarginValue: width * 0.05

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
			boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
			enabled: !_dialog.timePickerOnly
			Layout.topMargin: -5
			Layout.leftMargin: 5
			Layout.rightMargin: _dialog.titleBar.width
			Layout.fillWidth: true

			onChecked: (check) => mainTimer.stopWatch = check;
		}

		RowLayout {
			spacing: 20
			Layout.fillWidth: true
			Layout.leftMargin: _dialog.leftMarginValue

			TPTextInput {
				id: txtHours
				text: mainTimer.strHours
				backgroundOpacity: 0.9
				validator: IntValidator { bottom: 0; top: 99; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				horizontalAlignment: Text.AlignHCenter
				Layout.minimumWidth: _dialog.txtWidth
				Layout.maximumWidth: _dialog.txtWidth
				Layout.alignment: Qt.AlignCenter

				Keys.onPressed: (event) => _dialog.processKeyEvents(event);

				onActiveFocusChanged: {
					if (activeFocus) {
						txtHours.clear();
					} else {
						if (acceptableInput)
							mainTimer.strHours = text;
						else
							mainTimer.hours = mainTimer.orignalHours();
					}
				}

				onTextEdited: {
					if (acceptableInput) {
						if ( text.length === 2 ) {
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
				Layout.minimumWidth: _dialog.txtWidth
				Layout.maximumWidth: _dialog.txtWidth
				Layout.alignment: Qt.AlignCenter

				Keys.onPressed: (event) => _dialog.processKeyEvents(event);

				onActiveFocusChanged: {
					if (activeFocus) {
						txtMinutes.clear();
					} else {
						if (acceptableInput)
							mainTimer.strMinutes = text;
						else
							mainTimer.minutes = mainTimer.orignalMinutes();
					}
				}

				onTextEdited: {
					if (acceptableInput) {
						if (text.length === 2) {
							if (!_dialog.timePickerOnly) {
								txtSecs.focus = true;
								txtSecs.forceActiveFocus();
							}
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
				enabled: !_dialog.timePickerOnly
				validator: IntValidator { bottom: 0; top: 59; }
				inputMethodHints: Qt.ImhDigitsOnly
				maximumLength: 2
				horizontalAlignment: Text.AlignHCenter
				Layout.minimumWidth: _dialog.txtWidth
				Layout.maximumWidth: _dialog.txtWidth
				Layout.alignment: Qt.AlignCenter

				Keys.onPressed: (event) => _dialog.processKeyEvents(event);

				onActiveFocusChanged: {
					if (activeFocus) {
						txtSecs.clear();
					} else {
						if (acceptableInput)
							mainTimer.strSeconds = text;
						else
							mainTimer.seconds = mainTimer.orignalSeconds();
					}
				}
				onTextEdited: {
					if (acceptableInput) {
						if ( text.length === 2 ) {
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
			enabled: !_dialog.timePickerOnly
			from: mainTimer.totalSeconds
			to: 0
			value: mainTimer.progressValue
			indeterminate: chkStopWatch.checked
			Layout.fillWidth: true
			Layout.preferredWidth: 6
			Layout.rightMargin: _dialog.leftMarginValue
			Layout.leftMargin: _dialog.leftMarginValue

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
			Layout.leftMargin: !_dialog.timePickerOnly ? btnWidth / 2 : 5
			Layout.preferredWidth: _dialog.rowWidth

			readonly property int btnWidth: _dialog.width / 3 - 5

			TPButton {
				id: btnStartPause
				text: mainTimer.active ? qsTr("Pause") : mainTimer.paused ? qsTr("Continue") : qsTr("Start")
				visible: !_dialog.timePickerOnly
				enabled: mainTimer.stopWatch ? true : mainTimer.totalSeconds > 0
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
				enabled: !_dialog.timePickerOnly
				Layout.preferredWidth: btnsRow.btnWidth
				Layout.alignment: Qt.AlignHCenter

				onClicked: mainTimer.resetTimer(mainTimer.active);
			}

			TPButton {
				id: btnDone
				text: qsTr("Done")
				visible: _dialog.timePickerOnly
				Layout.alignment: Qt.AlignHCenter

				onClicked: {
					if (_dialog.timePickerOnly)
						_dialog.useTime(mainTimer.strHours + ":" + mainTimer.strMinutes);
					_dialog.closePopup(0);
				} //btnClose
			}
		} //Row
	} //ColumnLayout

	function processKeyEvents(event) {
		switch (event.key) {
		case Qt.Key_Enter:
		case Qt.Key_Return:
			if (!_dialog.timePickerOnly) {
				if (btnStartPause.enabled) {
					btnStartPause.forceActiveFocus();
					btnStartPause.clicked();
				}
			} else {
				_dialog.closePopup(-1);
			}
			break;
		}
	}
} // Dialog
