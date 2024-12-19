// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPopup {
	id: timePicker
	bKeepAbove: true
	width: appSettings.pageWidth * 0.78
	height: appSettings.pageHeight * 0.60
	x: (appSettings.pageWidth - width) / 2
	finalYPos: (appSettings.pageHeight - height) / 2
	topPadding: 0
	leftPadding: 0
	rightPadding: 0

	property bool isOK: false
	property int timeButtonsPaneSize: timePicker.width
	property int innerButtonsPaneSize: timeButtonsPaneSize - 50

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
	// this model used to display selected time
	// so you can add per ex. AM, PM or so
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

	// set these properties before start
	property int outerButtonIndex: 0
	property int innerButtonIndex: -1
	property bool bOnlyFutureTime: false
	property bool pickMinutes: false
	property bool useWorkTimes: true
	property bool onlyQuartersAllowed: false
	property bool autoSwapToMinutes: true

	property string hrsDisplay: "24"
	property string minutesDisplay: "00"

	signal timeSet(string hour, string minutes)

	// opening TimePicker with a given HH:MM value
	// ATTENTION TimePicker is rounding DOWN to next lower 05 / 15 Minutes
	// if you want to round UP do it before calling this function
	function setDisplay(hhmm, q, w): void {
		onlyQuartersAllowed = q;
		useWorkTimes = w;
		const s = hhmm.split(':');
		if (s.length === 2) {
			const hours = s[0];
			let minutes = s[1];
			minutes = minutes - onlyQuartersAllowed ? minutes%15 : minutes%5;
			showMinutes(minutes.toString());
			showHour(hours.toString());
			checkDisplay();
		}
	}

	function showHour(hour): void {
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
		console.log("Hour not found: "+hour)
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

	function showMinutes(minutes): void {
		for (let i=0; i < timePickerDisplayModel.length; i++) {
			const m = timePickerDisplayModel[i];
			if (m.m === minutes) {
				innerButtonIndex = -1;
				outerButtonIndex = i;
				pickMinutes = true;
				updateDisplayMinutes();
				return;
			}
		}
		// not found
		console.log("Minutes not found: "+minutes)
		innerButtonIndex = -1;
		outerButtonIndex = 0;
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

	Pane {
		id: headerPane
		padding: 0
		focus: true

		width: parent.width
		height: parent.height * 0.20
		background: Rectangle {
			color: appSettings.primaryColor
		}

		GridLayout {
			id: headerGrid
			Layout.fillWidth: true
			property int myPointSize: 24
			anchors.centerIn: parent
			rows: 2
			columns: 3
			rowSpacing: 0

			Button {
				id: hrsButton
				focusPolicy: Qt.NoFocus
				Layout.alignment: Text.AlignRight
				checked: true
				checkable: true
				contentItem: Label {
					text: timePicker.hrsDisplay
					font.pixelSize: headerGrid.myPointSize
					font.bold: true
					fontSizeMode: Text.Fit
					opacity: hrsButton.checked ? 1.0 : 0.6
					color: appSettings.fontColor
					elide: Text.ElideRight
				}
				background: Rectangle {
					color: "transparent"
				}
				onClicked: {
					onHoursButtonClicked();
				}
			} // hrsButton

			Label {
				text: ":"
				Layout.alignment: Text.AlignHCenter
				font.pixelSize: headerGrid.myPointSize
				font.bold: true
				fontSizeMode: Text.Fit
				opacity: 0.6
				color: appSettings.fontColor
			}

			Button {
				id: minutesButton
				focusPolicy: Qt.NoFocus
				Layout.alignment: Text.AlignLeft
				checked: false
				checkable: true
				contentItem: Label {
					text: timePicker.minutesDisplay
					font.pixelSize: headerGrid.myPointSize
					font.bold: true
					fontSizeMode: Text.Fit
					opacity: minutesButton.checked ? 1.0 : 0.6
					color: appSettings.fontColor
					elide: Text.ElideRight
				}
				background: Rectangle {
					color: "transparent"
				}
				onClicked: {
					onMinutesButtonClicked();
				}
			} // hrsButton
		} // header grid
	} // headerPane

	Pane {
		id: timeButtonsPane
		width: parent.width
		height: parent.height * 0.70
		padding: 0
		spacing: 1

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: headerPane.bottom
		anchors.topMargin: 10
		background: Rectangle {color: "transparent"}

		TPButton {
			text: qsTr("Now")
			flat: false

			anchors {
				top: parent.top
				topMargin: -5
				left: parent.left
				leftMargin: 5
			}

			onClicked: timePicker.setDisplay(appUtils.getCurrentTimeString(), timePicker.onlyQuartersAllowed, timePicker.useWorkTimes)
		}

		TPButton {
			visible: !timePicker.pickMinutes
			imageSource: timePicker.useWorkTimes? "work.png" : "time.png"
			hasDropShadow: false

			anchors {
				right: parent.right
				top: parent.top
				topMargin: -5
				rightMargin: 5
			}

			onClicked: {
				timePicker.useWorkTimes = !timePicker.useWorkTimes
				timePicker.showHour(timePicker.hrsDisplay)
			}
		}
		TPButton {
			visible: timePicker.pickMinutes
			text: timePicker.onlyQuartersAllowed? "15min" : "5min"

			anchors {
				right: parent.right
				top: parent.top
				topMargin: -5
				rightMargin: 5
			}

			onClicked: {
				timePicker.onlyQuartersAllowed = !timePicker.onlyQuartersAllowed
				timePicker.showMinutes(timePicker.minutesDisplay)
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
			background: Rectangle {color: "transparent"}

			Repeater {
				id: innerRepeater
				model: timePicker.timePickerModel
				delegate: Button {
					id: innerButton
					focusPolicy: Qt.NoFocus
					text: timePicker.useWorkTimes? modelData.n : modelData.c2
					font.bold: checked
					x: timePicker.innerButtonsPaneSize / 2 - width / 2 //- 20
					y: timePicker.innerButtonsPaneSize / 2 - height / 2 //- 20
					width: 40
					height: 40
					checked: index === timePicker.innerButtonIndex
					checkable: true
					enabled: buttonsIsEnabled(text, true);

					onClicked: {
						timePicker.outerButtonIndex = -1
						timePicker.innerButtonIndex = index
						if(timePicker.useWorkTimes) {
							timePicker.hrsDisplay = timePicker.timePickerDisplayModel[index].n
						} else {
							timePicker.hrsDisplay = timePicker.timePickerDisplayModel[index].c2
						}
						if(timePicker.autoSwapToMinutes) {
							timePicker.onMinutesButtonClicked()
						}
					}

					ButtonGroup.group: innerButtonGroup

					property real angle: 360 * (index / innerRepeater.count)

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

					contentItem: Label {
						text: innerButton.text
						font: innerButton.font
						minimumPointSize: 8
						fontSizeMode: Text.Fit
						opacity: innerButton.checked ? 1.0 : enabled || innerButton.highlighted ? 1.0 : 0.6
						color: appSettings.fontColor
						horizontalAlignment: Text.AlignHCenter
						verticalAlignment: Text.AlignVCenter
						rotation: -innerButton.angle
					} // content Label

					background: Rectangle {
						color: innerButton.checked ? appSettings.primaryColor : "transparent"
						radius: width / 2
					}
				} // inner button
			} // innerRepeater
		} // innerButtonsPane

		Repeater {
			id: outerRepeater
			model: timePicker.timePickerModel
			delegate: Button {
				id: outerButton
				focusPolicy: Qt.NoFocus
				text: timePicker.pickMinutes? modelData.m : timePicker.useWorkTimes? modelData.d : modelData.c1
				font.bold: true
				anchors.centerIn: parent
				width: 40
				height: 40
				checked: index === timePicker.outerButtonIndex
				checkable: true
				enabled: timePicker.pickMinutes ? (buttonsIsEnabled(text, false) ? timePicker.onlyQuartersAllowed ? modelData.q : true : false) :
							buttonsIsEnabled(text, true)

				onClicked: {
					timePicker.innerButtonIndex = -1
					timePicker.outerButtonIndex = index
					if(timePicker.pickMinutes) {
						timePicker.minutesDisplay = timePicker.timePickerDisplayModel[index].m
					} else {
						if(timePicker.useWorkTimes) {
							timePicker.hrsDisplay = timePicker.timePickerDisplayModel[index].d
						} else {
							timePicker.hrsDisplay = timePicker.timePickerDisplayModel[index].c1
						}
						if(timePicker.autoSwapToMinutes) {
							timePicker.onMinutesButtonClicked()
						}
					}
				}

				ButtonGroup.group: outerButtonGroup

				property real angle: 360 * (index / outerRepeater.count)

				transform: [
					Translate {
						y: -timePicker.timeButtonsPaneSize * 0.5 + outerButton.height / 2
					},
					Rotation {
						angle: outerButton.angle
						origin.x: outerButton.width / 2
						origin.y: outerButton.height / 2
					}
				]

				contentItem: Label {
					text: outerButton.text
					font: outerButton.font
					minimumPointSize: 10
					fontSizeMode: Text.Fit
					opacity: enabled || outerButton.highlighted || outerButton.checked ? 1 : 0.3
					color: appSettings.fontColor
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
					rotation: -outerButton.angle
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
			height: timePicker.timeButtonsPaneSize / 2 - 40
			transformOrigin: Item.Bottom
			rotation: outerButtonGroup.checkedButton ? outerButtonGroup.checkedButton.angle : 0
			color: appSettings.fontColor
		}

		Rectangle { // line to inner buttons
			visible: timePicker.innerButtonIndex >= 0 && !timePicker.pickMinutes
			y: centerpoint.y + centerpoint.height / 2 - height
			anchors.horizontalCenter: parent.horizontalCenter
			width: 1
			height: timePicker.innerButtonsPaneSize / 2 - 40
			transformOrigin: Item.Bottom
			rotation: innerButtonGroup.checkedButton ? innerButtonGroup.checkedButton.angle : 0
			color: appSettings.fontColor
		}

		Rectangle {
			id: centerpoint
			anchors.centerIn: parent
			width: 10
			height: 10
			color: appSettings.primaryColor
			radius: width / 2
		}
	} // timeButtonsPane

	onOpened: {
		timePicker.isOK = false
	}

	Pane {
		id: footerPane
		padding: 0
		width: parent.width
		height: parent.height * 0.10
		anchors.right: parent.right
		anchors.top: timeButtonsPane.bottom
		anchors.left: parent.left

		background: Rectangle {
			color: "transparent"
		}

		TPButton {
			text: qsTr("Cancel")
			flat: false

			anchors {
				top: parent.top
				left: parent.left
				leftMargin: 5
				topMargin: -10
			}

			onClicked: timePicker.close();
		}

		TPButton {
			text: qsTr("OK")
			flat: false

			anchors {
				top: parent.top
				right: parent.right
				rightMargin: 5
				topMargin: -10
			}

			onClicked: {
				timeSet(hrsDisplay, minutesDisplay);
				timePicker.isOK = true;
				timePicker.close();
			}
		}
	} // footer pane
} // timePicker

