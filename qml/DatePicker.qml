import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
	id: root
	clip: true
	height: cellSize * ( justCalendar ? 11 : 12.5 )
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

	Keys.onPressed: (event) => {
		if (event.key === Qt.Key_Back) {
			event.accepted = true;
			close();
		}
	}

	Component.onCompleted: {
		thisDay = new Date();
		root.forceActiveFocus();
	}

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

		Text {
			id: selectedYear
			text: calendar.currentYear
			leftPadding: cellSize * 0.5
			horizontalAlignment: Text.AlignLeft
			verticalAlignment: Text.AlignVCenter
			font.pointSize: fontSizePx * 2
			font.bold: true
			opacity: yearsList.visible ? 1 : 0.7
			color: AppSettings.fontColor
			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}

			MouseArea {
				anchors.fill: parent
				onClicked: yearsList.visible ? yearsList.hide() : yearsList.show();
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
		height: cellSize * 7.5
		visible: true
		z: 1
		snapMode: ListView.SnapToItem
		orientation: ListView.Horizontal
		spacing: cellSize

		anchors {
			top: titleOfDate.bottom
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
					radius: height * 0.5
					enabled: model.month === monthGrid.month
					gradient: Gradient {
						orientation: Gradient.Vertical
						GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
						GradientStop { position: 0.25; color: AppSettings.primaryColor; }
						GradientStop { position: 0.50; color: AppSettings.primaryDarkColor; }
						GradientStop { position: 0.75; color: AppSettings.primaryLightColor; }
					}

					readonly property bool highlighted: model.day === calendar.currentDay && model.month === calendar.currentMonth
					readonly property bool todayDate: model.year === thisDay.getFullYear() && model.month === thisDay.getMonth() && model.day === thisDay.getDate()

					Component.onCompleted: {
						var colorValue = "transparent";
						if ( highlighted )
							return AppSettings.primaryLightColor;
						else {
							if ( monthGrid.month === model.month )
								colorValue =  AppSettings.paneBackgroundColor;
						}
						color = colorValue
					}

					Text {
						anchors.centerIn: parent
						text: model.day
						font.pointSize: fontSizePx
						font.bold: true
						scale: highlighted ? 1.25 : 1
						Behavior on scale { NumberAnimation { duration: 150 } }
						visible: parent.enabled
						color: todayDate ? "red" : parent.highlighted ? "black" : "white"
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
		z: 2
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
				onClicked: {
					var nyear, nmonth, nday;
					nyear = yearsList.startYear + index;
					if (nyear === startDate.getFullYear()) {
						nmonth = startDate.getMonth();
						nday = startDate.getDate();
					}
					else {
						nmonth = endDate.getMonth();
						nday = endDate.getDate();
					}
					setDate(new Date(nyear, nmonth, nday));
					yearsList.hide();
				}
			}
		}

		Component.onCompleted: {
			for (var year = startYear; year <= endYear; year++) {
				yearsModel.append({name: year});
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


	Row {
		height: cellSize
		spacing: 20
		z: 1
		anchors {
			top: calendar.bottom
			right: parent.right
			rightMargin: cellSize * 0.5
		}

		TPButton {
			text: qsTr("CANCEL")
			visible: !justCalendar

			onClicked: cancelClicked();
		}

		TPButton {
			text: "OK"

			onClicked: {
				if (!justCalendar)
					okClicked(selectedDate);
				else
					cancelClicked();
			}
		}
	}

	function setDate(newDate) {
		selectedDate = newDate;
		calendar.currentIndex = calendarModel.indexOf(selectedDate);
		calendar.positionViewAtIndex(calendar.currentIndex, ListView.SnapPosition);
		calendar.currentDay = displayDate.getDate();
		calendar.currentMonth = displayDate.getMonth();
		calendar.currentYear = displayDate.getFullYear();
		calendar.dayOfWeek = displayDate.getDay();
	}
}
