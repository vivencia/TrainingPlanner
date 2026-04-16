pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml

Rectangle {
	id: _control
	clip: true
	height: _control.cellSize * 10.5
	width: _control.cellSize * 8
	radius: 10

//public:
	required property CalendarModel calendarModel
	property date startDate
	property date endDate
	property date selectedDate

	signal dateSelected(date selDate)

//private:
	readonly property double _size_factor: 7
	readonly property date thisDay: new Date()
	readonly property double cellSize: Screen.pixelDensity * _size_factor
	readonly property int fontSizePx: _control.cellSize * (_size_factor / 21) //0.32

	TPBackRec {
		id: titleOfDate
		useGradient: true
		radius: 10
		opacity: 0.8
		z: 1
		height: 2.5 * _control.cellSize
		width: parent.width

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
		}

		TextField {
			id: selectedYear
			text: _control.selectedDate.getUTCFullYear()
			leftPadding: _control.cellSize * 0.5
			horizontalAlignment: Text.AlignLeft
			verticalAlignment: Text.AlignVCenter
			font.pixelSize: _control.fontSizePx * 2
			font.bold: true
			readOnly: true
			inputMethodHints: Qt.ImhDigitsOnly
			validator: IntValidator { id: val; bottom: _control.startDate.getUTCFullYear(); top: _control.endDate.getUTCFullYear(); }
			opacity: yearsList.visible ? 1 : 0.7
			color: AppSettings.fontColor
			z: 1

			property bool yearOK

			background: Rectangle {
				height: _control.cellSize * 2
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
						_control.yearChosen(parseInt(text));
						_control.showHideMonthsList();
					}
					break;
				case Qt.Key_Escape:
					event.accepted = true;
					_control.showHideYearsList();
					break;
				default: return;
				}
			}

			onPressed: _control.showHideYearsList();

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
						yearsModel.append({"name": year});
				}
			}
		}

		TextField {
			id: txtSelectedWeekDayAndMonth
			text: AppUtils.dayName(_control.selectedDate.getUTCDay()).slice(0, 3) + ", " + _control.selectedDate.getUTCDate() +
															" " + AppUtils.monthName(_control.selectedDate.getUTCMonth()).slice(0, 3)
			leftPadding: _control.cellSize * 0.5
			horizontalAlignment: Text.AlignLeft
			verticalAlignment: Text.AlignVCenter
			font.pixelSize: height * 0.5
			font.bold: true
			readOnly: true
			color: AppSettings.fontColor
			opacity: monthsList.visible ? 0.7 : 1
			height: _control.cellSize * 2
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
					_control.monthChosen(monthsList.currentIndex);
					break;
				case Qt.Key_Escape:
					event.accepted = true;
					txtSelectedWeekDayAndMonth.readOnly = true;
					monthsList.hide();
				break;
				default: return;
				}
			}

			onPressed: _control.showHideMonthsList();

			function filterInput(): void {
				monthsModel.clear();
				let monthOK = false;
				for (let i = 0; i < 12; ++i) {
					if (_control.selectedDate.getUTCFullYear() === _control.startDate.getUTCFullYear()) {
						if (i < _control.startDate.getUTCMonth())
							continue;
					}
					if (_control.selectedDate.getUTCFullYear() === _control.endDate.getUTCFullYear()) {
						if (i > _control.startDate.getUTCMonth())
							continue;
					}
					if (text.length === 0)
						monthsModel.append({name: AppUtils.monthName(i)});
					else {
						const found = AppUtils.monthName(i).toLowerCase().indexOf(text.toLowerCase()) >= 0;
						if (found) {
							monthsModel.append({name: AppUtils.monthName(i)});
							if (!monthOK) {
								monthOK = true;
								monthsList.currentIndex = i;
							}
						}
					}
				}
				if (!monthOK)
					monthsList.currentIndex = _control.selectedDate.getUTCMonth();
			}
		}
	} //titleOfDate

	ListView {
		id: calendar
		visible: true
		z: 1
		snapMode: ListView.SnapToItem
		reuseItems: true
		orientation: ListView.Horizontal
		spacing: _control.cellSize
		model: _control.calendarModel

		anchors {
			top: titleOfDate.bottom
			bottom: parent.bottom
			left: parent.left
			right: parent.right
			leftMargin: _control.cellSize * 0.5
			rightMargin: _control.cellSize * 0.5
		}

		delegate: Rectangle {
			id: cal_delegate
			height: _control.cellSize * 7
			width: _control.cellSize * 7

			required property var model

			Rectangle {
				id: monthYearTitle
				height: _control.cellSize
				width: parent.width
				anchors.top: parent.top

				Text {
					text: AppUtils.monthName(cal_delegate.model.month) + " " + cal_delegate.model.year;
					font.pixelSize: _control.fontSizePx * 1.2
					font.bold: true
					anchors.centerIn: parent
				}
			}

			DayOfWeekRow {
				id: weekTitles
				locale: monthGrid.locale
				anchors.top: monthYearTitle.bottom
				height: _control.cellSize
				width: parent.width

				delegate: Text {
					text: model.shortName
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
					font.pixelSize: _control.fontSizePx
					font.bold: true

					required property var model
				}
			}

			MonthGrid {
				id: monthGrid
				month: cal_delegate.model.month
				year: cal_delegate.model.year
				spacing: 0
				locale: Qt.locale(AppSettings.userLocale)
				width: _control.cellSize * 7
				height: _control.cellSize * 6
				anchors.top: weekTitles.bottom

				delegate: TPCalendarEntry {
					id: _delegate
					entryDay: day
					entryMonth: month
					entryYear: year
					cellSize: _control.cellSize
					today: _control.thisDay
					parentMonth: monthGrid.month
					qtCalendarModel: _control.calendarModel
					selectedDate: _control.selectedDate

					required property int day
					required property int month
					required property int year

					onDateSelected: (day, month, year, is_workout) => {
						_control.selectedDate = new Date(year, month, day);
						_control.dateSelected(_control.selectedDate);
					}

				} // delegate
			} // MonthGrid
		} // model: CalendarModel
	} // ListView calendar

	ListView {
		id: yearsList
		visible: false
		z: 1
		anchors.fill: calendar

		property int currentYear: _control.startDate.getUTCFullYear()
		readonly property int startYear: _control.startDate.getUTCFullYear()
		readonly property int endYear : _control.endDate.getUTCFullYear()

		model: ListModel {
			id: yearsModel
		}

		delegate: TPBackRec {
			id: years_delegate
			useGradient: true
			opacity: 0.8
			width: yearsList.width
			height: _control.cellSize * 1.5

			required property int index
			required property string name

			Text {
				anchors.centerIn: parent
				font.pixelSize: _control.fontSizePx * 1.5
				text: years_delegate.name
				scale: years_delegate.index === yearsList.currentYear - yearsList.startYear ? 1.5 : 1
				color: AppSettings.fontColor
			}
			MouseArea {
				anchors.fill: parent
				onClicked: _control.yearChosen(years_delegate.name);
			}
		}

		function show(): void {
			visible = true;
			calendar.visible = false;
			currentYear = _control.selectedDate.getUTCFullYear();
			yearsList.positionViewAtIndex(currentYear - _control.startDate.setUTCFullYear, ListView.SnapToItem);
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

		property int currentMonth: _control.startDate.getUTCMonth()

		model: ListModel {
			id: monthsModel
		}

		delegate: TPBackRec {
			id: months_delegate
			useGradient: true
			width: monthsList.width
			height: _control.cellSize * 1.5
			opacity: 0.8

			required property int index
			required property string name

			Text {
				anchors.centerIn: parent
				font.pixelSize: _control.fontSizePx * 1.5
				text: months_delegate.name
				font.family: "FreeMono"
				scale: months_delegate.index === monthsList.currentMonth ? 1.5 : 1
				color: AppSettings.fontColor

				MouseArea {
					anchors.fill: parent
					onClicked: _control.monthChosen(months_delegate.index);
				}
			}
		}

		function show(): void {
			visible = true;
			calendar.visible = false;
			currentMonth = _control.selectedDate.getUTCMonth();
			monthsList.positionViewAtIndex(currentMonth, ListView.SnapToItem);
		}

		function hide(): void {
			visible = false;
			calendar.visible = true;
		}
	} // ListView monthsList

	function daysInMonth(for_date: date): int{
		return new Date(for_date.getUTCFullYear(), for_date.getUTCMonth(), 0).getUTCDate();
	 }

	function showHideYearsList(): void {
		if (yearsList.visible) {
			selectedYear.text = Qt.binding(function() { return _control.selectedDate.getUTCFullYear(); });
			selectedYear.readOnly = true;
			yearsList.hide();
		}
		else {
			selectedYear.readOnly = false;
			selectedYear.clear();
			selectedYear.forceActiveFocus();
			selectedYear.filterInput();
			yearsList.show();
		}
	}

	function showHideMonthsList(): void {
		if (monthsList.visible) {
			txtSelectedWeekDayAndMonth.text = Qt.binding(function() {
				return AppUtils.dayName(_control.selectedDate.getUTCDay()).slice(0, 3) + ", " + _control.selectedDate.getUTCDate() +
														" " + AppUtils.monthName(_control.selectedDate.getUTCMonth()).slice(0, 3); });
			txtSelectedWeekDayAndMonth.readOnly = true;
			monthsList.hide();
			_control.forceActiveFocus();
		}
		else {
			txtSelectedWeekDayAndMonth.readOnly = false;
			txtSelectedWeekDayAndMonth.clear();
			txtSelectedWeekDayAndMonth.forceActiveFocus();
			txtSelectedWeekDayAndMonth.filterInput();
			monthsList.show();
		}
	}

	function yearChosen(year: int): void {
		setDate(new Date(year, selectedDate.getUTCMonth(), selectedDate.getUTCDate()));
		showHideYearsList();
	}

	function monthChosen(month: int): void {
		setDate(new Date(selectedDate.getUTCFullYear(), month, selectedDate.getUTCDate()));
		showHideMonthsList();
	}

	function setDate(newDate): void {
		//Postion the ListView in the correct view ahead of changing selectedDate. This way, the delegates are created first
		//so they can respond to the change of the selectedDate property
		calendar.currentIndex = calendarModel.indexOf(newDate);
		calendar.positionViewAtIndex(calendar.currentIndex, ListView.SnapPosition);
		selectedDate = newDate;
		dateSelected(selectedDate);
	}

	function setDate2(newDate): void {
		newDate.setDate(newDate.getUTCDate()+1);
		setDate(newDate);
	}

	function setDateByTyping(key1: int, key2: int): void {
		let day = -1;
		switch (key1) {
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
		if (key2 === -1) {
			if (day >= 1) {
				selectedDate = new Date(selectedDate.getUTCFullYear(), selectedDate.getUTCMonth(), day);
				dateSelected(selectedDate);
			}
		}
		else {
			let typed_day = selectedDate.getUTCDate();
			switch (typed_day) {
			case 0:
			case 1:
			case 2:
				typed_day *= 10;
				typed_day += day;
				break;
			case 3:
				typed_day *= 10;
				typed_day += day;
				if (typed_day > daysInMonth(selectedDate))
					typed_day = -1;
				break;
			default: typed_day = -1;
			}
			if (typed_day >= 1) {
				selectedDate = new Date(selectedDate.getUTCFullYear(), selectedDate.getUTCMonth(), typed_day);
				dateSelected(selectedDate);
			}
		}
	}
}
