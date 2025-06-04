import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../"
import "../TPWidgets"

Rectangle {
	id: root
	clip: true
	height: cellSize * 10.5
	width: cellSize * 8
	radius: 10

	required property var calendarModel
	property date displayDate
	property date startDate
	property date endDate
	property date selectedDate: displayDate

	property double sizeFactor: 7
	readonly property date thisDay: new Date()
	readonly property double cellSize: Screen.pixelDensity * sizeFactor
	readonly property int fontSizePx: cellSize * (sizeFactor/21) //0.32
	readonly property list<string> months_names: [qsTr("January"), qsTr("February"), qsTr("March"), qsTr("April"), qsTr("May"), qsTr("June"), qsTr("July"),
		qsTr("August"), qsTr("September"), qsTr("October"), qsTr("November"), qsTr("December")]

	signal dateSelected(date selDate)

	Rectangle {
		id: titleOfDate
		color: appSettings.paneBackgroundColor
		radius: 10
		opacity: 0.8
		z: 1
		height: 2.5 * cellSize
		width: parent.width

		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
			GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
			GradientStop { position: 0.50; color: appSettings.primaryColor; }
			GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
		}

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
		}

		TextField {
			id: selectedYear
			text: selectedDate.getFullYear()
			leftPadding: cellSize * 0.5
			horizontalAlignment: Text.AlignLeft
			verticalAlignment: Text.AlignVCenter
			font.pixelSize: fontSizePx * 2
			font.bold: true
			readOnly: true
			inputMethodHints: Qt.ImhDigitsOnly
			validator: IntValidator { id: val; bottom: startDate.getFullYear(); top: endDate.getFullYear(); }
			opacity: yearsList.visible ? 1 : 0.7
			color: appSettings.fontColor
			z: 1

			property bool yearOK

			background: Rectangle {
				height: cellSize * 2
				width: parent.width
				border.color: "transparent"
				color: "transparent"
			}

			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}

			onTextEdited: filterInput();

			Keys.onPressed: (event) => {
				switch (event.key) {
					case Qt.Key_Enter:
					case Qt.Key_Return:
						if (selectedYear.yearOK) {
							event.accepted = true;
							yearChosen(parseInt(text));
							showHideMonthsList();
						}
					break;
					default: return;
				}
			}

			onPressed: {
				if (yearsList.visible) {
					if (text.length === 0)
						text = Qt.binding(function() { return selectedDate.getFullYear(); });
					readOnly = true;
					yearsList.hide();
				}
				else {
					readOnly = false;
					clear();
					forceActiveFocus();
					filterInput();
					yearsList.show();
				}
			}

			function filterInput(): void {
				yearsModel.clear();
				let topYear, bottomYear;
				yearOK = false;
				switch (text.length) {
					case 1: bottomYear = parseInt(text) * 1000; topYear = bottomYear + 999; break;
					case 2: bottomYear = parseInt(text) * 100; topYear = bottomYear + 99; break;
					case 3: bottomYear = parseInt(text) * 10; topYear = bottomYear + 9; break;
					case 4: bottomYear = topYear = parseInt(text); yearOK = true; break;
					default: bottomYear = val.bottom; topYear = val.top; break;
				}
				for (let year = val.bottom; year <= val.top; year++) {
					if (year >= bottomYear && year <= topYear)
						yearsModel.append({name: year});
				}
			}
		}

		TextField {
			id: selectedWeekDayMonth
			text: calendar.weekNames[selectedDate.getDay()].slice(0, 3) + ", " + selectedDate.getDate() + " " + calendar.months[selectedDate.getMonth()].slice(0, 3)
			leftPadding: cellSize * 0.5
			horizontalAlignment: Text.AlignLeft
			verticalAlignment: Text.AlignVCenter
			font.pixelSize: height * 0.5
			font.bold: true
			readOnly: true
			color: appSettings.fontColor
			opacity: monthsList.visible ? 0.7 : 1
			height: cellSize * 2
			z: 1

			background: Rectangle {
				border.color: "transparent"
				color: "transparent"
			}

			anchors {
				left: parent.left
				right: parent.right
				top: selectedYear.bottom
				bottom: parent.bottom
			}

			onTextEdited: filterInput();

			Keys.onPressed: (event) => {
				switch (event.key) {
					case Qt.Key_Enter:
					case Qt.Key_Return:
						event.accepted = true;
						monthChosen(monthsList.currentIndex);
					break;
					default: return;
				}
			}

			onPressed: showHideMonthsList();

			function filterInput(): void {
				monthsModel.clear();
				let monthOK = false;
				for (let i = 0; i < 12; ++i) {
					if (selectedDate.getFullYear() === startDate.getFullYear()) {
						if (i < startDate.getMonth())
							continue;
					}
					if (selectedDate.getFullYear() === endDate.getFullYear()) {
						if (i > startDate.getMonth())
							continue;
					}
					if (text.length === 0)
						monthsModel.append({name: months_names[i]});
					else {
						const found = months_names[i].toLowerCase().indexOf(text.toLowerCase()) >= 0;
						if (found) {
							monthsModel.append({name: months_names[i]});
							if (!monthOK) {
								monthOK = true;
								monthsList.currentIndex = i;
							}
						}
					}
				}
				if (!monthOK)
					monthsList.currentIndex = selectedDate.getMonth();
			}
		}
	} //titleOfDate

	ListView {
		id: calendar
		visible: true
		z: 1
		snapMode: ListView.SnapToItem
		orientation: ListView.Horizontal
		spacing: cellSize
		model: calendarModel

		Connections {
			target: calendarModel ? calendarModel : null
			ignoreUnknownSignals: true

			function onReadyChanged() {
				calendar.currentIndex = calendarModel.indexOf(selectedDate);
				calendar.positionViewAtIndex(calendar.currentIndex, ListView.SnapPosition);
				dateSelected(selectedDate);
			}
		}

		anchors {
			top: titleOfDate.bottom
			bottom: parent.bottom
			left: parent.left
			right: parent.right
			leftMargin: cellSize * 0.5
			rightMargin: cellSize * 0.5
		}

		readonly property list<string> months: [qsTr("January"), qsTr("February"), qsTr("March"), qsTr("April"),
									qsTr("May"), qsTr("June"), qsTr("July"), qsTr("August"),
									qsTr("September"), qsTr("October"), qsTr("November"), qsTr("December")]
		readonly property list<string> weekNames: [qsTr("Sunday"), qsTr("Monday"), qsTr("Tuesday"), qsTr("Wednesday"),
									qsTr("Thursday"), qsTr("Friday"), qsTr("Saturday")]

		delegate: Rectangle {
			height: cellSize * 7
			width: cellSize * 7

			Rectangle {
				id: monthYearTitle
				anchors.top: parent.top
				height: cellSize
				width: parent.width

				Text {
					anchors.centerIn: parent
					font.pixelSize: fontSizePx * 1.2
					font.bold: true
					text: calendar.months[model.month] + " " + model.year;
				}
			}

			DayOfWeekRow {
				id: weekTitles
				locale: monthGrid.locale
				anchors.top: monthYearTitle.bottom
				height: cellSize
				width: parent.width

				delegate: Text {
					text: model.shortName
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
					font.pixelSize: fontSizePx
					font.bold: true
				}
			}


			MonthGrid {
				id: monthGrid
				month: model.month
				year: model.year
				spacing: 0
				locale: Qt.locale(appSettings.appLocale)
				width: cellSize * 7
				height: cellSize * 6
				anchors.top: weekTitles.bottom

				delegate: Rectangle {
					height: cellSize
					width: cellSize
					radius: cellSize * 0.5
					opacity: monthGrid.month === model.month ? 1 : 0.5
					color: appSettings.primaryColor

					readonly property bool highlighted: model.day === selectedDate.getDate() && model.month === selectedDate.getMonth()
					readonly property bool todayDate: model.year === thisDay.getFullYear() && model.month === thisDay.getMonth() && model.day === thisDay.getDate()

					Text {
						text: model.day
						font.pixelSize: fontSizePx
						font.bold: true
						scale: highlighted ? 1.25 : 1
						Behavior on scale { NumberAnimation { duration: 150 } }
						color: todayDate ? "red" : parent.highlighted ? "green" : monthGrid.month === model.month ? appSettings.fontColor : appSettings.disabledFontColor
						anchors.centerIn: parent
					}

					MouseArea {
						anchors.fill: parent
						onClicked: {
							selectedDate = new Date(model.year, model.month, model.day);
							dateSelected(selectedDate);
						}
					}
				} // delegate: Rectangle
			} // MonthGrid
		} // model: CalendarModel
	} // ListView calendar

	ListView {
		id: yearsList
		visible: false
		z: 1
		anchors.fill: calendar

		property int currentYear: startDate.getFullYear()
		readonly property int startYear: startDate.getFullYear()
		readonly property int endYear : endDate.getFullYear()

		model: ListModel {
			id: yearsModel
		}

		delegate: Rectangle {
			width: yearsList.width
			height: cellSize * 1.5
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8

			Text {
				anchors.centerIn: parent
				font.pixelSize: fontSizePx * 1.5
				text: name
				scale: index === yearsList.currentYear - yearsList.startYear ? 1.5 : 1
				color: appSettings.fontColor
			}
			MouseArea {
				anchors.fill: parent
				onClicked: yearChosen(yearsModel.get(index).name);
			}
		}

		function show(): void {
			visible = true;
			calendar.visible = false;
			currentYear = selectedDate.getFullYear();
			yearsList.positionViewAtIndex(currentYear - startYear, ListView.SnapToItem);
		}

		function hide(): void {
			visible = false;
			calendar.visible = true;
		}
	} // ListView yearsList

	ListView {
		id: monthsList
		visible: false
		z: 1
		anchors.fill: calendar

		property int currentMonth: startDate.getMonth()

		model: ListModel {
			id: monthsModel
		}

		delegate: Rectangle {
			width: monthsList.width
			height: cellSize * 1.5
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8

			Text {
				anchors.centerIn: parent
				font.pixelSize: fontSizePx * 1.5
				text: name
				scale: index === monthsList.currentMonth ? 1.5 : 1
				color: appSettings.fontColor

				MouseArea {
					anchors.fill: parent
					onClicked: monthChosen(index);
				}
			}
		}

		function show(): void {
			visible = true;
			calendar.visible = false;
			currentMonth = selectedDate.getMonth();
			monthsList.positionViewAtIndex(currentMonth, ListView.SnapToItem);
		}

		function hide(): void {
			visible = false;
			calendar.visible = true;
		}
	} // ListView monthsList

	function daysInMonth(for_date: date): int{
		return new Date(for_date.getFullYear(), for_date.getMonth(), 0).getDate();
	 }

	function showHideMonthsList(): void {
		if (monthsList.visible) {
			if (selectedWeekDayMonth.text.length === 0)
				selectedWeekDayMonth.text = Qt.binding(function() {
					return calendar.weekNames[selectedDate.getDay()].slice(0, 3) + ", " + selectedDate.getDate() +
									" " + calendar.months[selectedDate.getMonth()].slice(0, 3); });
			selectedWeekDayMonth.readOnly = true;
			monthsList.hide();
			calendar.forceActiveFocus();
		}
		else {
			selectedWeekDayMonth.readOnly = false;
			selectedWeekDayMonth.clear();
			selectedWeekDayMonth.forceActiveFocus();
			selectedWeekDayMonth.filterInput();
			monthsList.show();
		}
	}

	function yearChosen(year: int): void {
		setDate(new Date(year, selectedDate.getMonth(), selectedDate.getDate()));
		selectedYear.readOnly = true;
		yearsList.hide();
	}

	function monthChosen(month: int): void {
		setDate(new Date(selectedDate.getFullYear(), month, selectedDate.getDate()));
		selectedWeekDayMonth.readOnly = true;
		monthsList.hide();
	}

	function setDate(newDate): void {
		selectedDate = newDate;
		dateSelected(selectedDate);
		calendar.currentIndex = calendarModel.indexOf(selectedDate);
		calendar.positionViewAtIndex(calendar.currentIndex, ListView.SnapPosition);
	}

	function setDate2(newDate): void {
		newDate.setDate(newDate.getDate()+1);
		setDate(newDate);
	}

	function setDateByTyping(key: int): void {
		let typed_day = -1;
		let day = -1;
		switch (key) {
			case Qt.Key_0: day = 0; break;
			case Qt.Key_1: day = 1; break;
			case Qt.Key_2: day = 2; break;
			case Qt.Key_3: day = 3; break;
			case Qt.Key_4: day = 4; break;
			case Qt.Key_5: day = 5; break;
			case Qt.Key_6: day = 6; break;
			case Qt.Key_7: day = 7; break;
			case Qt.Key_8: day = 8; break;
			case Qt.Key_9: day = 9; break;
			default: return;
		}
		if (day >= 0)
		{
			if (typed_day == -1)
				typed_day = day;
			else
			{
				if (typed_day >= 10)
					typed_day = day;
				else {
					switch (typed_day) {
						case 0:
						case 1:
						case 2:
							typed_day *= 20;
							typed_day += day;
						break;
						case 3:
							typed_day *= 20;
							typed_day += day;
							if (typed_day > daysInMonth(selectedDate))
								typed_day = -1;
						break;
						default: typed_day = -1;
					}
				}
			}
			if (typed_day >= 1)
			{
				selectedDate = new Date(selectedDate.getFullYear(), selectedDate.getMonth(), typed_day);
				dateSelected(selectedDate);
			}
		}
	}
}
