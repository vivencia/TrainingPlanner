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

	required property date displayDate
	required property date startDate
	required property date endDate

	property date selectedDate: displayDate
	property bool justCalendar: false

	property date thisDay
	readonly property double cellSize: Screen.pixelDensity * 7
	property int fontSizePx: cellSize * 0.32

	signal okClicked(date selDate)
	signal cancelClicked

	Component.onCompleted: thisDay = new Date();

	Rectangle {
		id: titleOfDate
		height: 2.5 * cellSize
		width: parent.width
		color: AppSettings.paneBackgroundColor
		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
			GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
			GradientStop { position: 0.50; color: AppSettings.primaryColor; }
			GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
		}
		opacity: 0.8
		z: 1

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
		}

		TextField {
			id: selectedYear
			text: calendar.currentYear
			leftPadding: cellSize * 0.5
			horizontalAlignment: Text.AlignLeft
			verticalAlignment: Text.AlignVCenter
			font.pointSize: fontSizePx * 2
			font.bold: true
			readOnly: true
			inputMethodHints: Qt.ImhDigitsOnly
			validator: IntValidator { id: val; bottom: startDate.getFullYear(); top: endDate.getFullYear(); }
			opacity: yearsList.visible ? 1 : 0.7
			color: AppSettings.fontColor
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
						}
					break;
					default: return;
				}
			}

			onPressed: {
				if (yearsList.visible) {
					if (text.length === 0)
						text = Qt.binding(function() { return calendar.currentYear; });
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

			function filterInput() {
				yearsModel.clear();
				var topYear, bottomYear;
				yearOK = false;
				switch (text.length) {
					case 1: bottomYear = parseInt(text) * 1000; topYear = bottomYear + 999; break;
					case 2: bottomYear = parseInt(text) * 100; topYear = bottomYear + 99; break;
					case 3: bottomYear = parseInt(text) * 10; topYear = bottomYear + 9; break;
					case 4: bottomYear = topYear = parseInt(text); yearOK = true; break;
					default: bottomYear = val.bottom; topYear = val.top; break;
				}
				for (var year = val.bottom; year <= val.top; year++) {
					if (year >= bottomYear && year <= topYear)
						yearsModel.append({name: year});
				}
			}
		}

		Text {
			id: selectedWeekDayMonth
			leftPadding: cellSize * 0.5
			verticalAlignment: Text.AlignVCenter
			font.pointSize: height * 0.5
			font.bold: true
			text: calendar.weekNames[calendar.dayOfWeek].slice(0, 3) + ", " + calendar.currentDay + " " + calendar.months[calendar.currentMonth].slice(0, 3)
			color: AppSettings.fontColor
			opacity: yearsList.visible ? 0.7 : 1

			anchors {
				left: parent.left
				right: parent.right
				top: selectedYear.bottom
				bottom: parent.bottom
			}

			MouseArea {
				anchors.fill: parent
				onClicked: yearsList.hide();
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

		anchors {
			top: titleOfDate.bottom
			bottom: parent.bottom
			left: parent.left
			right: parent.right
			leftMargin: cellSize * 0.5
			rightMargin: cellSize * 0.5
		}

		model: CalendarModel {
			id: calendarModel
			from: startDate
			to: endDate
		}

		property int currentDay
		property int currentMonth
		property int currentYear
		property int dayOfWeek
		readonly property var months: [qsTr("January"), qsTr("February"), qsTr("March"), qsTr("April"),
									qsTr("May"), qsTr("June"), qsTr("July"), qsTr("August"),
									qsTr("September"), qsTr("October"), qsTr("November"), qsTr("December")]
		readonly property var weekNames: [qsTr("Sunday"), qsTr("Monday"), qsTr("Tuesday"), qsTr("Wednesday"),
									qsTr("Thursday"), qsTr("Friday"), qsTr("Saturday")]

		delegate: Rectangle {
			height: cellSize * 7
			width: cellSize * 7
			Rectangle {
				id: monthYearTitle
				anchors.top: parent.top
				height: cellSize * 1.3
				width: parent.width

				Text {
					anchors.centerIn: parent
					font.pointSize: fontSizePx * 1.2
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
					font.pointSize: fontSizePx
					font.bold: true
				}
			}

			MonthGrid {
				id: monthGrid
				month: model.month
				year: model.year
				spacing: 0
				anchors.top: weekTitles.bottom
				width: cellSize * 7
				height: cellSize * 6

				locale: Qt.locale(AppSettings.appLocale)
				delegate: Rectangle {
					height: cellSize
					width: cellSize
					radius: cellSize * 0.5
					opacity: monthGrid.month === model.month ? 1 : 0.5

					gradient: Gradient {
						orientation: Gradient.Vertical
						GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
						GradientStop { position: 0.25; color: AppSettings.primaryColor; }
						GradientStop { position: 0.50; color: AppSettings.primaryDarkColor; }
						GradientStop { position: 0.75; color: AppSettings.primaryLightColor; }
					}

					readonly property bool highlighted: model.day === calendar.currentDay && model.month === calendar.currentMonth
					readonly property bool todayDate: model.year === thisDay.getFullYear() && model.month === thisDay.getMonth() && model.day === thisDay.getDate()

					Text {
						anchors.centerIn: parent
						text: model.day
						font.pointSize: fontSizePx
						font.bold: true
						scale: highlighted ? 1.25 : 1
						Behavior on scale { NumberAnimation { duration: 150 } }
						color: todayDate ? "red" : parent.highlighted ? "black" : monthGrid.month === model.month ? "white" : "gray"
					}
					MouseArea {
						anchors.fill: parent
						onClicked: {
							selectedDate = new Date(model.year, model.month, model.day);
							calendar.currentYear = model.year;
							calendar.currentMonth = model.month;
							calendar.currentDay = model.day;
							calendar.dayOfWeek = selectedDate.getDay();
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

		property int currentYear
		readonly property int startYear: startDate.getFullYear();
		readonly property int endYear : endDate.getFullYear();

		model: ListModel {
			id: yearsModel
		}

		delegate: Rectangle {
			width: yearsList.width
			height: cellSize * 1.5
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8

			Text {
				anchors.centerIn: parent
				font.pointSize: fontSizePx * 1.5
				text: name
				scale: index === yearsList.currentYear - yearsList.startYear ? 1.5 : 1
				color: AppSettings.fontColor
			}
			MouseArea {
				anchors.fill: parent
				onClicked: yearChosen(yearsModel.get(index).name);
			}
		}

		function show() {
			visible = true;
			calendar.visible = false
			currentYear = calendar.currentYear
			yearsList.positionViewAtIndex(currentYear - startYear, ListView.SnapToItem);
		}

		function hide() {
			visible = false;
			calendar.visible = true;
		}
	} // ListView yearsList

	function yearChosen(year: int) {
		setDate(new Date(year, showDate.getMonth(), showDate.getDate()));
		selectedYear.readOnly = true;
		yearsList.hide();
	}

	function setDate(newDate) {
		selectedDate = newDate;
		calendar.currentIndex = calendarModel.indexOf(selectedDate);
		calendar.positionViewAtIndex(calendar.currentIndex, ListView.SnapPosition);
		calendar.currentDay = selectedDate.getDate();
		calendar.currentMonth = selectedDate.getMonth();
		calendar.currentYear = selectedDate.getFullYear();
		calendar.dayOfWeek = selectedDate.getDay();
	}
}
