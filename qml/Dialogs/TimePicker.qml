// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPopup {
	id: timePicker
	keepAbove: true
	width: appSettings.pageWidth * 0.6
	height: appSettings.pageHeight * 0.5
	x: (appSettings.pageWidth - width) / 2
	finalYPos: (appSettings.pageHeight - height) / 2
	padding: 0

	property int outerButtonIndex: 0
	property int innerButtonIndex: -1
	property bool bOnlyFutureTime: false
	property bool pickMinutes: false
	property bool useWorkTimes: true
	property bool onlyQuartersAllowed: false
	property bool autoSwapToMinutes: true

	property string hrsDisplay: "24"
	property string minutesDisplay: "00"
	property bool  modified: false
	property int timeButtonsPaneSize: timePicker.width
	property int innerButtonsPaneSize: timeButtonsPaneSize - 60

	// c1: circle 1 (outside) 1-12
	// c2: circle 2 (inside) 13-00
	// d: work day (outside) 7-18
	// n: night (inside) 19-6
	// m: minutes
	// q: only quarters allowed for minutes, disable the other ones
	// data model used to display labels on picker circles
	readonly property var timePickerModel: [
		{"c1":"12","c2":"00","d":"12","n":"00","m":"00","q":true},
		{"c1":"1","c2":"13","d":"13","n":"1","m":"05","q":false},
		{"c1":"2","c2":"14","d":"14","n":"2","m":"10","q":false},
		{"c1":"3","c2":"15","d":"15","n":"3","m":"15","q":true},
		{"c1":"4","c2":"16","d":"16","n":"4","m":"20","q":false},
		{"c1":"5","c2":"17","d":"17","n":"5","m":"25","q":false},
		{"c1":"6","c2":"18","d":"18","n":"6","m":"30","q":true},
		{"c1":"7","c2":"19","d":"7","n":"19","m":"35","q":false},
		{"c1":"8","c2":"20","d":"8","n":"20","m":"40","q":false},
		{"c1":"9","c2":"21","d":"9","n":"21","m":"45","q":true},
		{"c1":"10","c2":"22","d":"10","n":"22","m":"50","q":false},
		{"c1":"11","c2":"23","d":"11","n":"23","m":"55","q":false}
	]
	// this model used to display selected time so you can add per ex. AM, PM or so
	readonly property var timePickerDisplayModel: [
		{"c1":"12","c2":"00","d":"12","n":"00","m":"00","q":true},
		{"c1":"01","c2":"13","d":"13","n":"01","m":"05","q":false},
		{"c1":"02","c2":"14","d":"14","n":"02","m":"10","q":false},
		{"c1":"03","c2":"15","d":"15","n":"03","m":"15","q":true},
		{"c1":"04","c2":"16","d":"16","n":"04","m":"20","q":false},
		{"c1":"05","c2":"17","d":"17","n":"05","m":"25","q":false},
		{"c1":"06","c2":"18","d":"18","n":"06","m":"30","q":true},
		{"c1":"07","c2":"19","d":"07","n":"19","m":"35","q":false},
		{"c1":"08","c2":"20","d":"08","n":"20","m":"40","q":false},
		{"c1":"09","c2":"21","d":"09","n":"21","m":"45","q":true},
		{"c1":"10","c2":"22","d":"10","n":"22","m":"50","q":false},
		{"c1":"11","c2":"23","d":"11","n":"23","m":"55","q":false}
	]

	signal timeSet(string hour, string minutes)

	onOpened: modified = false;

	// opening TimePicker with a given HH:MM value
	// ATTENTION TimePicker is rounding DOWN to next lower 05 / 15 Minutes. If you want to round UP do it before calling this function
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
		for (let i = 0; i < timePickerDisplayModel.length; i++) {
			const h = timePickerDisplayModel[i];
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
			hrsDisplay = useWorkTimes ? timePickerDisplayModel[innerButtonIndex].n : timePickerDisplayModel[innerButtonIndex].c2;
		else
			hrsDisplay = useWorkTimes ? timePickerDisplayModel[outerButtonIndex].d : timePickerDisplayModel[outerButtonIndex].c1;
	}

	function findMinutes(minutes: string): int {
		for (let i = 0; i < timePickerDisplayModel.length; i++) {
			const m = timePickerDisplayModel[i];
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
		minutesDisplay = timePickerDisplayModel[outerButtonIndex].m;
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
		timePicker.pickMinutes = true;
		timePicker.showMinutes(timePicker.minutesDisplay);
	}

	function onHoursButtonClicked(): void {
		minutesButton.checked = false;
		hrsButton.checked = true;
		timePicker.pickMinutes = false;
		timePicker.showHour(timePicker.hrsDisplay);
	}

	function buttonsIsEnabled(buttonText: string, bHour: bool): bool {
		if (bOnlyFutureTime) {
			const curHour = parseInt(appUtils.getHourFromCurrentTime());
			const buttonTextValue = parseInt(buttonText);
			if (bHour)
				return buttonTextValue >= curHour;
			else {
				const hsrDisplayValue = parseInt(hrsDisplay);
				if (hsrDisplayValue === curHour)
					return buttonTextValue >= parseInt(appUtils.getMinutesFromCurrentTime())
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
			top: parent.top
			left: parent.left
			right: parent.right
		}

		TPButton {
			id: hrsButton
			text: timePicker.hrsDisplay
			autoSize: true
			checked: !minutesButton.checked
			checkable: true
			Layout.alignment: Qt.AlignRight

			onCheck: onHoursButtonClicked();
		} // hrsButton

		TPLabel {
			text: ":"
			Layout.alignment: Qt.AlignHCenter
		}

		TPButton {
			id: minutesButton
			text: timePicker.minutesDisplay
			autoSize: true
			checked: !hrsButton.checked
			checkable: true
			Layout.alignment: Qt.AlignLeft
			onCheck: onMinutesButtonClicked();
		} // minutesButton
	} // headerPane

	Rectangle {
		id: timeButtonsPane
		color: appSettings.primaryDarkColor
		height: parent.height * 0.70
		radius: width / 2

		anchors {
			top: headerPane.bottom
			topMargin: 10
			left: parent.left
			right: parent.right
		}

		TPButton {
			text: qsTr("Now")
			width: appSettings.itemLargeHeight
			z: 2

			anchors {
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: -(height + 10)
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: {
				modified = true;
				timePicker.setDisplay(appUtils.getCurrentTimeString(), timePicker.onlyQuartersAllowed)
			}
		}

		TPButton {
			text: timePicker.onlyQuartersAllowed? "15min" : "5min"
			width: appSettings.itemLargeHeight
			visible: timePicker.pickMinutes
			z: 2

			anchors {
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: height + 10
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: {
				timePicker.onlyQuartersAllowed = !timePicker.onlyQuartersAllowed
				timePicker.showMinutes(timePicker.minutesDisplay)
			}
		}

		TPButton {
			imageSource: timePicker.useWorkTimes? "work.png" : "time.png"
			width: appSettings.itemDefaultHeight
			height: width
			visible: !timePicker.pickMinutes
			z: 2

			anchors {
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: height + 10
				horizontalCenter: parent.horizontalCenter
			}

			onClicked: {
				timePicker.useWorkTimes = !timePicker.useWorkTimes
				timePicker.showHour(timePicker.hrsDisplay)
			}
		}

		ButtonGroup {
			id: outerButtonGroup
		}
		ButtonGroup {
			id: innerButtonGroup
		}

		Pane {
			id: innerButtonsPane
			implicitWidth: timePicker.innerButtonsPaneSize
			implicitHeight: timePicker.innerButtonsPaneSize
			padding: 0
			visible: !timePicker.pickMinutes
			anchors.centerIn: parent
			background: Rectangle {color: appSettings.primaryLightColor; radius: width / 2; }

			Repeater {
				id: innerRepeater
				model: timePicker.timePickerModel

				delegate: Button {
					id: innerButton
					focusPolicy: Qt.NoFocus
					text: timePicker.useWorkTimes? modelData.n : modelData.c2
					font.bold: checked
					x: (timePicker.innerButtonsPaneSize - width) / 2
					y: (timePicker.innerButtonsPaneSize - height) / 2
					ButtonGroup.group: innerButtonGroup
					width: timePicker.width * 0.11
					height: timePicker.height * 0.11
					checked: index === timePicker.innerButtonIndex
					checkable: true
					enabled: buttonsIsEnabled(text, true);

					readonly property real angle: 360 * (index / innerRepeater.count)

					contentItem: Item {}
					TPLabel {
						text: innerButton.text
						rotation: -innerButton.angle
						opacity: innerButton.checked ? 1.0 : enabled || innerButton.highlighted ? 1.0 : 0.6
						horizontalAlignment: Text.AlignHCenter
						anchors.centerIn: parent
					} // content Label

					background: Rectangle {
						color: innerButton.checked ? appSettings.primaryColor : "transparent"
						radius: width / 2
					}

					onClicked: {
						if (timePicker.innerButtonIndex !== index)
						{
							timePicker.outerButtonIndex = -1;
							timePicker.innerButtonIndex = index;
							if (timePicker.useWorkTimes)
								timePicker.hrsDisplay = timePicker.timePickerDisplayModel[index].n;
							else
								timePicker.hrsDisplay = timePicker.timePickerDisplayModel[index].c2;
							if (timePicker.autoSwapToMinutes)
								timePicker.onMinutesButtonClicked();
							modified = true;
						}
					}

					transform: [
						Translate {
							y: -timePicker.innerButtonsPaneSize * 0.5 + innerButton.height / 2
						},
						Rotation {
							angle: innerButton.angle
							origin.x: innerButton.width / 2
							origin.y: innerButton.height / 2
						}
					]
				} // inner button
			} // innerRepeater

		} // innerButtonsPane

		Repeater {
			id: outerRepeater
			model: timePicker.timePickerModel
			delegate: Button {
				id: outerButton
				focusPolicy: Qt.NoFocus
				text: timePicker.pickMinutes ? modelData.m : timePicker.useWorkTimes ? modelData.d : modelData.c1
				font.bold: true
				ButtonGroup.group: outerButtonGroup
				anchors.centerIn: parent
				width: timePicker.width * 0.11
				height: timePicker.height * 0.11
				checked: index === timePicker.outerButtonIndex
				checkable: true
				enabled: timePicker.pickMinutes ? (buttonsIsEnabled(text, false) ? timePicker.onlyQuartersAllowed ? modelData.q : true : false) :
							buttonsIsEnabled(text, true)

				readonly property real angle: 360 * (index / outerRepeater.count)

				onClicked: {
					if (timePicker.outerButtonIndex !== index) {
						timePicker.outerButtonIndex = index;
						timePicker.innerButtonIndex = -1;
						if(timePicker.pickMinutes)
							timePicker.minutesDisplay = timePicker.timePickerDisplayModel[index].m;
						else {
							if(timePicker.useWorkTimes)
								timePicker.hrsDisplay = timePicker.timePickerDisplayModel[index].d;
							else
								timePicker.hrsDisplay = timePicker.timePickerDisplayModel[index].c1;
						}
						if(timePicker.autoSwapToMinutes)
							timePicker.onMinutesButtonClicked()
						modified = true;
					}
				}

				transform: [
					Translate {
						y: -timePicker.timeButtonsPaneSize * 0.5 + outerButton.height / 2 - 5
					},
					Rotation {
						angle: outerButton.angle
						origin.x: outerButton.width / 2
						origin.y: outerButton.height / 2
					}
				]

				contentItem: Item {}
				TPLabel {
					text: outerButton.text
					rotation: -outerButton.angle
					opacity: enabled || outerButton.highlighted || outerButton.checked ? 1 : 0.3
					verticalAlignment: Text.AlignVCenter
					anchors.centerIn: parent
				} // outer content label

				background: Rectangle {
					color: outerButton.checked ? appSettings.primaryColor : "transparent"
					radius: width / 2
				}
			} // outer button
		} // outerRepeater

		Rectangle { // line to outer buttons
			visible: timePicker.outerButtonIndex >= 0
			y: centerpoint.y + centerpoint.height / 2 - height //timePicker.timeButtonsPaneSize / 2 - 40
			anchors.horizontalCenter: parent.horizontalCenter
			width: 1
			height: timePicker.timeButtonsPaneSize / 2 - appSettings.itemDefaultHeight
			transformOrigin: Item.Bottom
			rotation: outerButtonGroup.checkedButton ? outerButtonGroup.checkedButton.angle : 0
			color: appSettings.fontColor

			Rectangle { //tip point of line
				width: 8
				height: 8
				color: appSettings.fontColor
				radius: 4
				anchors {
					left: parent.left
					leftMargin: -4
					top: parent.top
				}
			}
		}

		Rectangle { // line to inner buttons
			visible: timePicker.innerButtonIndex >= 0 && !timePicker.pickMinutes
			y: centerpoint.y + centerpoint.height / 2 - height
			anchors.horizontalCenter: parent.horizontalCenter
			width: 1
			height: timePicker.innerButtonsPaneSize / 2 - appSettings.itemDefaultHeight
			transformOrigin: Item.Bottom
			rotation: innerButtonGroup.checkedButton ? innerButtonGroup.checkedButton.angle : 0
			color: appSettings.fontColor

			Rectangle { //tip point of line
				width: 6
				height: 6
				color: appSettings.fontColor
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
			color: appSettings.fontColor
			radius: 7.5
		}
	} // timeButtonsPane

	Item {
		id: footerPane
		height: parent.height * 0.10

		anchors {
			bottom: parent.bottom
			bottomMargin: 10
			left: parent.left
			right: parent.right
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

			onClicked: timePicker.closePopup();
		}

		TPButton {
			id: btnOK
			text: qsTr("OK")
			autoSize: true
			enabled: modified

			anchors {
				right: parent.right
				rightMargin: 10
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				timeSet(hrsDisplay, minutesDisplay);
				timePicker.closePopup();
			}
		}
	} // footer pane
} // timePicker
