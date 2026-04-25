pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

import "./TimePickerElements"

TPPopup {
	id: _timePicker
	keepAbove: true
	open_in_window: false
	width: AppSettings.pageWidth * 0.6
	height: AppSettings.pageHeight * 0.5
	padding: 0

	onOpened: modified = false;

//public:
	property bool bOnlyFutureTime: false
	property bool pickMinutes: false
	property bool useWorkTimes: true
	property bool onlyQuartersAllowed: false
	property bool autoSwapToMinutes: true

	signal timeSet(string hour, string minutes);

//private:
	property int outerButtonIndex: 0
	property int innerButtonIndex: -1
	property string hrsDisplay: "24"
	property string minutesDisplay: "00"
	property bool  modified: false
	property int timeButtonsPaneSize: _timePicker.width
	property int innerButtonsPaneSize: timeButtonsPaneSize - 60

	// c1: circle 1 (outside) 1-12
	// c2: circle 2 (inside) 13-00
	// d: work day (outside) 7-18
	// n: night (inside) 19-6
	// m: minutes
	// q: only quarters allowed for minutes, disable the other ones
	// data model used to display labels on picker circles
	readonly property var _timePickerModel: [
		{ "c1": "12", "c2": "00", "d": "12", "n": "00", "m": "00", "q": true },
		{ "c1": "1", "c2": "13", "d": "13", "n": "1", "m": "05", "q": false },
		{ "c1": "2", "c2": "14", "d": "14", "n": "2", "m": "10", "q": false },
		{ "c1": "3", "c2": "15", "d": "15", "n": "3", "m": "15", "q": true },
		{ "c1": "4", "c2": "16", "d": "16", "n": "4", "m": "20", "q": false },
		{ "c1": "5", "c2": "17", "d": "17", "n": "5", "m": "25", "q": false },
		{ "c1": "6", "c2": "18", "d": "18", "n": "6", "m": "30", "q": true },
		{ "c1": "7", "c2": "19", "d": "7", "n": "19", "m": "35", "q": false },
		{ "c1": "8", "c2": "20", "d": "8", "n": "20", "m": "40", "q": false },
		{ "c1": "9", "c2": "21", "d": "9", "n": "21", "m": "45", "q": true },
		{ "c1": "10", "c2": "22", "d": "10", "n": "22", "m": "50", "q": false },
		{ "c1": "11", "c2": "23", "d": "11", "n": "23", "m": "55", "q": false }
	]
	// this model is used to display selected time so you can add per ex. AM,  PM or so
	readonly property var _timePickerDisplayModel: [
		{ "c1": "12", "c2": "00", "d": "12", "n": "00", "m": "00", "q": true },
		{ "c1": "01", "c2": "13", "d": "13", "n": "01", "m": "05", "q": false },
		{ "c1": "02", "c2": "14", "d": "14", "n": "02", "m": "10", "q": false },
		{ "c1": "03", "c2": "15", "d": "15", "n": "03", "m": "15", "q": true },
		{ "c1": "04", "c2": "16", "d": "16", "n": "04", "m": "20", "q": false },
		{ "c1": "05", "c2": "17", "d": "17", "n": "05", "m": "25", "q": false },
		{ "c1": "06", "c2": "18", "d": "18", "n": "06", "m": "30", "q": true },
		{ "c1": "07", "c2": "19", "d": "07", "n": "19", "m": "35", "q": false },
		{ "c1": "08", "c2": "20", "d": "08", "n": "20", "m": "40", "q": false },
		{ "c1": "09", "c2": "21", "d": "09", "n": "21", "m": "45", "q": true },
		{ "c1": "10", "c2": "22", "d": "10", "n": "22", "m": "50", "q": false },
		{ "c1": "11", "c2": "23", "d": "11", "n": "23", "m": "55", "q": false }
	]

	// opening _timePicker with a given HH:MM value
	// ATTENTION _timePicker is rounding DOWN to next lower 05 / 15 Minutes. If you want to round UP do it before calling this function
	function setDisplay(hhmm: string, q: bool): void {
		onlyQuartersAllowed = q;
		const s = hhmm.split(':');
		if (s.length === 2) {
			const hours = s[0];
			let minutes = s[1];
			if (findMinutes(minutes) === 0) {
				const add_min = onlyQuartersAllowed ? 15 : 5;
				const wrong_mins = parseInt(minutes);
				for (let minute = 0; minute < 59; minute += add_min) {
					if (wrong_mins - minute < 5) {
						if (wrong_mins - minute <= onlyQuartersAllowed ? 7 : 2)
							minute += add_min;
						minutes = minute > 9 ? String(minute) : "0" + String(minute);
						break;
					}
				}
			}
			showMinutes(minutes.toString());
			showHour(hours.toString());
			checkDisplay();
		}
	}

	function showHour(hour: string): void {
		for (let i = 0; i < _timePickerDisplayModel.length; i++) {
			const h = _timePickerDisplayModel[i];
			if (useWorkTimes) {
				if (h.d === hour) {
					pickMinutes = false;
					innerButtonIndex = -1;
					outerButtonIndex = i;
					updateDisplayHour();
					return;
				}
				if (h.n === hour) {
					pickMinutes = false;
					outerButtonIndex = -1;
					innerButtonIndex = i;
					updateDisplayHour();
					return;
				}
			}
			else {
				if (h.c1 === hour) {
					pickMinutes = false;
					innerButtonIndex = -1;
					outerButtonIndex = i;
					updateDisplayHour();
					return;
				}
				if (h.c2 === hour) {
					pickMinutes = false;
					outerButtonIndex = -1;
					innerButtonIndex = i;
					updateDisplayHour();
					return;
				}
			}
		}
		// not found
		pickMinutes = false;
		innerButtonIndex = -1;
		outerButtonIndex = 0;
		updateDisplayHour();
	}

	function updateDisplayHour(): void {
		if (innerButtonIndex >= 0)
			hrsDisplay = useWorkTimes ? _timePickerDisplayModel[innerButtonIndex].n : _timePickerDisplayModel[innerButtonIndex].c2;
		else
			hrsDisplay = useWorkTimes ? _timePickerDisplayModel[outerButtonIndex].d : _timePickerDisplayModel[outerButtonIndex].c1;
	}

	function findMinutes(minutes: string): int {
		for (let i = 0; i < _timePickerDisplayModel.length; i++) {
			const m = _timePickerDisplayModel[i];
			if (m.m === minutes)
				return i;
		}
		return 0;
	}

	function showMinutes(minutes: string): void {
		innerButtonIndex = -1;
		outerButtonIndex = findMinutes(minutes);
		pickMinutes = true;
		updateDisplayMinutes();
	} // showMinutes

	function updateDisplayMinutes(): void {
		minutesDisplay = _timePickerDisplayModel[outerButtonIndex].m;
	}

	function checkDisplay(): void {
		if (pickMinutes) {
			hrsButton.checked = false;
			minutesButton.checked = true;
		}
		else {
			minutesButton.checked = false;
			hrsButton.checked = true;
		}
	}

	function onMinutesButtonClicked(): void {
		hrsButton.checked = false;
		minutesButton.checked = true;
		_timePicker.pickMinutes = true;
		_timePicker.showMinutes(_timePicker.minutesDisplay);
	}

	function onHoursButtonClicked(): void {
		minutesButton.checked = false;
		hrsButton.checked = true;
		_timePicker.pickMinutes = false;
		_timePicker.showHour(_timePicker.hrsDisplay);
	}

	function buttonsIsEnabled(buttonText: string, bHour: bool): bool {
		if (bOnlyFutureTime) {
			const curHour = parseInt(AppUtils.getHourFromCurrentTime());
			const buttonTextValue = parseInt(buttonText);
			if (bHour)
				return buttonTextValue >= curHour;
			else {
				const hsrDisplayValue = parseInt(hrsDisplay);
				if (hsrDisplayValue === curHour)
					return buttonTextValue >= parseInt(AppUtils.getMinutesFromCurrentTime())
				return hsrDisplayValue > curHour;
			}
		}
		return true;
	}

	RowLayout {
		id: headerPane
		spacing: 5
		z: 1

		anchors {
			top: _timePicker.contentItem.top
			left: _timePicker.contentItem.left
			right: _timePicker.contentItem.right
		}

		TPButton {
			id: hrsButton
			text: _timePicker.hrsDisplay
			autoSize: true
			checked: !minutesButton.checked
			checkable: true
			Layout.alignment: Qt.AlignRight

			onCheck: _timePicker.onHoursButtonClicked();
		} // hrsButton

		TPLabel {
			text: ":"
			Layout.alignment: Qt.AlignHCenter
		}

		TPButton {
			id: minutesButton
			text: _timePicker.minutesDisplay
			autoSize: true
			checked: !hrsButton.checked
			checkable: true
			Layout.alignment: Qt.AlignLeft
			onCheck: _timePicker.onMinutesButtonClicked();
		} // minutesButton
	} // headerPane

	Rectangle {
		id: timeButtonsPane
		color: AppSettings.primaryDarkColor
		height: _timePicker.height * 0.70
		radius: width / 2

		anchors {
			top: headerPane.bottom
			topMargin: 10
			left: _timePicker.contentItem.left
			right: _timePicker.contentItem.right
		}

		TPButton {
			text: qsTr("Now")
			width: AppSettings.itemLargeHeight
			z: 2

			anchors {
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: -(height + 10)
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: {
				_timePicker.modified = true;
				_timePicker.setDisplay(AppUtils.getCurrentTimeString(), _timePicker.onlyQuartersAllowed)
			}
		}

		TPButton {
			text: _timePicker.onlyQuartersAllowed? "15min" : "5min"
			width: AppSettings.itemLargeHeight
			visible: _timePicker.pickMinutes
			z: 2

			anchors {
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: height + 10
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: {
				_timePicker.onlyQuartersAllowed = !_timePicker.onlyQuartersAllowed
				_timePicker.showMinutes(_timePicker.minutesDisplay)
			}
		}

		TPButton {
			imageSource: _timePicker.useWorkTimes? "work.png" : "time.png"
			width: AppSettings.itemDefaultHeight
			height: width
			visible: !_timePicker.pickMinutes
			z: 2

			anchors {
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: height + 10
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: {
				_timePicker.useWorkTimes = !_timePicker.useWorkTimes
				_timePicker.showHour(_timePicker.hrsDisplay)
			}
		}

		ButtonGroup {
			id: outerButtonGroup
			readonly property ClockButton checkedClockButton: checkedButton as ClockButton
		}
		ButtonGroup {
			id: innerButtonGroup
			readonly property ClockButton checkedClockButton: checkedButton as ClockButton
		}

		Pane {
			id: innerButtonsPane
			implicitWidth: _timePicker.innerButtonsPaneSize
			implicitHeight: _timePicker.innerButtonsPaneSize
			padding: 0
			visible: !_timePicker.pickMinutes
			anchors.centerIn: parent
			background: Rectangle {color: AppSettings.primaryLightColor; radius: width / 2; }

			Repeater {
				id: innerRepeater
				model: _timePicker._timePickerModel

				delegate: ClockButton {
					nButtonsInGroup: innerRepeater.count
					indexInGroup: index
					buttonGroupSize: _timePicker.innerButtonsPaneSize
					text: _timePicker.useWorkTimes? _timePicker._timePickerModel.n : _timePicker._timePickerModel.c2
					x: (_timePicker.innerButtonsPaneSize - width) / 2
					y: (_timePicker.innerButtonsPaneSize - height) / 2
					width: _timePicker.width * 0.11
					height: _timePicker.height * 0.11
					checked: index === _timePicker.innerButtonIndex
					enabled: _timePicker.buttonsIsEnabled(text, true);
					ButtonGroup.group: innerButtonGroup

					required property int index

					onClicked: {
						if (_timePicker.innerButtonIndex !== index)
						{
							_timePicker.outerButtonIndex = -1;
							_timePicker.innerButtonIndex = index;
							if (_timePicker.useWorkTimes)
								_timePicker.hrsDisplay = _timePicker._timePickerDisplayModel[index].n;
							else
								_timePicker.hrsDisplay = _timePicker._timePickerDisplayModel[index].c2;
							if (_timePicker.autoSwapToMinutes)
								_timePicker.onMinutesButtonClicked();
							_timePicker.modified = true;
						}
					}
				} // inner button
			} // innerRepeater

		} // innerButtonsPane

		Repeater {
			id: outerRepeater
			model: _timePicker._timePickerModel
			delegate: ClockButton {
				text: _timePicker.pickMinutes ? _timePicker._timePickerModel.m : _timePicker.useWorkTimes ?
																	_timePicker._timePickerModel.d : _timePicker._timePickerModel.c1
				nButtonsInGroup: outerRepeater.count
				indexInGroup: index
				buttonGroupSize: _timePicker.timeButtonsPaneSize
				width: _timePicker.width * 0.11
				height: _timePicker.height * 0.11
				checked: index === _timePicker.outerButtonIndex
				checkable: true
				enabled: _timePicker.pickMinutes ? (_timePicker.buttonsIsEnabled(text, false) ? _timePicker.onlyQuartersAllowed ?
													_timePicker._timePickerModel.q : true : false) : _timePicker.buttonsIsEnabled(text, true)
				ButtonGroup.group: outerButtonGroup
				anchors.centerIn: parent

				required property int index

				onClicked: {
					if (_timePicker.outerButtonIndex !== index) {
						_timePicker.outerButtonIndex = index;
						_timePicker.innerButtonIndex = -1;
						if(_timePicker.pickMinutes)
							_timePicker.minutesDisplay = _timePicker._timePickerDisplayModel[index].m;
						else {
							if(_timePicker.useWorkTimes)
								_timePicker.hrsDisplay = _timePicker._timePickerDisplayModel[index].d;
							else
								_timePicker.hrsDisplay = _timePicker._timePickerDisplayModel[index].c1;
						}
						if(_timePicker.autoSwapToMinutes)
							_timePicker.onMinutesButtonClicked()
						_timePicker.modified = true;
					}
				}
			} // outer button
		} // outerRepeater

		Rectangle { // line to outer buttons
			visible: _timePicker.outerButtonIndex >= 0
			y: centerpoint.y + centerpoint.height / 2 - height //_timePicker.timeButtonsPaneSize / 2 - 40
			anchors.horizontalCenter: parent.horizontalCenter
			width: 1
			height: _timePicker.timeButtonsPaneSize / 2 - AppSettings.itemDefaultHeight
			transformOrigin: Item.Bottom
			rotation: outerButtonGroup.checkedClockButton ? outerButtonGroup.checkedClockButton.angle : 0
			color: AppSettings.fontColor

			Rectangle { //tip point of line
				width: 8
				height: 8
				color: AppSettings.fontColor
				radius: 4
				anchors {
					left: parent.left
					leftMargin: -4
					top: parent.top
				}
			}
		}

		Rectangle { // line to inner buttons
			visible: _timePicker.innerButtonIndex >= 0 && !_timePicker.pickMinutes
			y: centerpoint.y + centerpoint.height / 2 - height
			anchors.horizontalCenter: parent.horizontalCenter
			width: 1
			height: _timePicker.innerButtonsPaneSize / 2 - AppSettings.itemDefaultHeight
			transformOrigin: Item.Bottom
			rotation: innerButtonGroup.checkedClockButton ? innerButtonGroup.checkedClockButton.angle : 0
			color: AppSettings.fontColor

			Rectangle { //tip point of line
				width: 6
				height: 6
				color: AppSettings.fontColor
				radius: 3
				anchors {
					left: parent.left
					leftMargin: -3
					top: parent.top
				}
			}
		}

		Rectangle {
			id: centerpoint
			anchors.centerIn: parent
			width: 15
			height: 15
			color: AppSettings.fontColor
			radius: 7.5
		}
	} // timeButtonsPane

	Item {
		id: footerPane
		height: _timePicker.height * 0.10

		anchors {
			bottom: _timePicker.contentItem.bottom
			bottomMargin: 10
			left: _timePicker.contentItem.left
			right: _timePicker.contentItem.right
		}

		TPButton {
			id: btnCancel
			text: qsTr("Cancel")
			autoSize: true

			anchors {
				left: parent.left
				leftMargin: 10
				verticalCenter: parent.verticalCenter
			}

			onClicked: _timePicker.closePopup();
		}

		TPButton {
			id: btnOK
			text: qsTr("OK")
			autoSize: true
			enabled: _timePicker.modified

			anchors {
				right: parent.right
				rightMargin: 10
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				_timePicker.timeSet(_timePicker.hrsDisplay, _timePicker.minutesDisplay);
				_timePicker.closePopup();
			}
		}
	} // footer pane
} // _timePicker
