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
	property string calendarWindowTitle

	property date thisDay
	property double mm: Screen.pixelDensity
	property double cellSize: mm * 7
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
		id: windowTitelBar
		color: primaryLightColor
		visible: calendarWindowTitle.length > 0
		width: parent.width
		height: lblTitle.contentHeight + 10

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
		}

		Label {
			id: lblTitle
			anchors.centerIn: parent
			text: calendarWindowTitle
			wrapMode: Text.WordWrap
			width: parent.width
			horizontalAlignment: Text.AlignJustify
			verticalAlignment: Text.AlignJustify
			font.bold: true
		}
	}

	Rectangle {
		id: titleOfDate
		anchors {
			top: windowTitelBar.visible ? windowTitelBar.bottom : parent.top
			horizontalCenter: parent.horizontalCenter
		}
		height: 2.5 * cellSize
		width: parent.width
		color: paneBackgroundColor
		z: 2
		Rectangle {
			id: selectedYear
			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}
			height: cellSize * 1
			color: parent.color
			Text {
				id: yearTitle
				anchors.fill: parent
				leftPadding: cellSize * 0.5
				topPadding: cellSize * 0.5
				horizontalAlignment: Text.AlignLeft
				verticalAlignment: Text.AlignVCenter
				font.pixelSize: fontSizePx * 1.7
				font.bold: true
				opacity: yearsList.visible ? 1 : 0.7
				color: "white"
				text: calendar.currentYear
			}
			MouseArea {
				anchors.fill: parent
				onClicked: {
					yearsList.show();
				}
			}
		}
		Text {
			id: selectedWeekDayMonth
			anchors {
				left: parent.left
				right: parent.right
				top: selectedYear.bottom
				bottom: parent.bottom
			}
			leftPadding: cellSize * 0.5
			verticalAlignment: Text.AlignVCenter
			font.pixelSize: height * 0.5
			font.bold: true
			text: calendar.weekNames[calendar.dayOfWeek].slice(0, 3) + ", " + calendar.currentDay + " " + calendar.months[calendar.currentMonth].slice(0, 3)
			color: "white"
			opacity: yearsList.visible ? 0.7 : 1
			MouseArea {
				anchors.fill: parent
				onClicked: {
					yearsList.hide();
				}
			}
		}
	}

	ListView {
		id: calendar
		anchors {
			top: titleOfDate.bottom
			left: parent.left
			right: parent.right
			leftMargin: cellSize * 0.5
			rightMargin: cellSize * 0.5
		}
		height: cellSize * 8
		visible: true
		z: 1
		snapMode: ListView.SnapToItem
		orientation: ListView.Horizontal
		spacing: cellSize

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
			height: cellSize * 8.5
			width: cellSize * 7
			Rectangle {
				id: monthYearTitle
				anchors {
					top: parent.top
				}
				height: cellSize * 1.3
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
				anchors {
					top: monthYearTitle.bottom
				}
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
				anchors {
					top: weekTitles.bottom
				}
				width: cellSize * 7
				height: cellSize * 6

				locale: Qt.locale(AppSettings.appLocale)
				delegate: Rectangle {
					height: cellSize
					width: cellSize
					radius: height * 0.5
					enabled: model.month === monthGrid.month
					color: highlighted ? paneBackgroundColor : "white"

					readonly property bool highlighted: model.day === calendar.currentDay && model.month === calendar.currentMonth
					readonly property bool todayDate: model.year === thisDay.getFullYear() && model.month === thisDay.getMonth() && model.day === thisDay.getDate()

					Text {
						anchors.centerIn: parent
						text: model.day
						font.pixelSize: fontSizePx
						font.bold: true
						scale: highlighted ? 1.25 : 1
						Behavior on scale { NumberAnimation { duration: 150 } }
						visible: parent.enabled
						color: todayDate ? "red" : parent.highlighted ? "white" : "black"
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
		anchors.fill: calendar
		orientation: ListView.Vertical
		visible: false
		z: calendar.z

		property int currentYear
		property int startYear: startDate.getFullYear();
		property int endYear : endDate.getFullYear();
		model: ListModel {
			id: yearsModel
		}

		delegate: Rectangle {
			width: yearsList.width
			height: cellSize * 1.5
			Text {
				anchors.centerIn: parent
				font.pixelSize: fontSizePx * 1.5
				text: name
				scale: index === yearsList.currentYear - yearsList.startYear ? 1.5 : 1
				color: paneBackgroundColor
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

	Rectangle {
		height: cellSize
		anchors {
			top: calendar.bottom
			right: parent.right
			rightMargin: cellSize * 0.5
			topMargin: -20
		}
		z: titleOfDate.z
		color: "black"
		Row {
			layoutDirection: "RightToLeft"
			anchors {
				right: parent.right
			}
			height: parent.height

			Rectangle {
				id: okBtn
				height: parent.height
				width: okBtnText.contentWidth + cellSize
				Text {
					id: okBtnText
					anchors.centerIn: parent
					font.pixelSize: fontSizePx * 1.8
					font.bold: true
					color: "black"
					text: "OK"
				}
				MouseArea {
					anchors.fill: parent
					onClicked: {
						if (!justCalendar)
							okClicked(selectedDate);
						else
							cancelClicked();
					}
				}
			}
			Rectangle {
				id: cancelBtn
				height: parent.height
				width: cancelBtnText.contentWidth + cellSize
				visible: !justCalendar
				Text {
					id: cancelBtnText
					anchors.centerIn: parent
					font.pixelSize: fontSizePx * 1.8
					font.bold: true
					color: "black"
					text: qsTr("CANCEL")
				}
				MouseArea {
					anchors.fill: parent
					onClicked: {
						cancelClicked();
					}
				}
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
