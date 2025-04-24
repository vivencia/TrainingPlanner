import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"
import "../TPWidgets"

TPPage {
	id: mesoCalendarPage
	objectName: "mesoCalendarPage"

	required property CalendarManager calendarManager
	required property DBCalendarModel calendarModel

	property date _today
	property bool bAlreadyLoaded: false

	header: TPToolBar {
		height: appSettings.pageHeight*0.1

		ColumnLayout {
			anchors.fill: parent
			spacing: 0

			TPLabel {
				id: lbl1
				text: calendarManager.nameLabel
				font: AppGlobals.extraLargeFont
				widthAvailable: parent.width*0.8
				singleLine: true
				Layout.maximumWidth: widthAvailable
				Layout.alignment: Qt.AlignCenter
			}
			TPLabel {
				id: lbl2
				text: calendarManager.dateLabel
				font: AppGlobals.regularFont
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: parent.width - 10
				Layout.preferredHeight: heightAvailable
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				Layout.bottomMargin: 5
			}
		}
	}

	ListView {
		id: calendar
		model: calendarModel
		snapMode: ListView.SnapToItem
		spacing: 2
		anchors.fill: parent

		readonly property double mm: Screen.pixelDensity
		readonly property double cellSize: mm * 7
		readonly property int fontSizePx: calendar.cellSize * 0.32
		readonly property var monthsNames: [qsTr("January"), qsTr("February"), qsTr("March"), qsTr("April"),
									qsTr("May"), qsTr("June"), qsTr("July"), qsTr("August"),
									qsTr("September"), qsTr("October"), qsTr("November"), qsTr("December")]

		property date dayInfoDate
		property int currentDay
		property int currentMonth
		property int currentYear

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true//ScrollBar.AlwaysOn
		}

		delegate: Rectangle {
			height: calendar.cellSize * 10.5
			width: calendar.width - 10
			color: appSettings.primaryDarkColor
			opacity: 0.7

			Rectangle {
				id: monthYearTitle
				anchors {
					top: parent.top
				}
				height: calendar.cellSize * 1.3
				width: parent.width

				Text {
					anchors.centerIn: parent
					text: calendar.monthsNames[calendarModel.getMonth(index)] + " " + calendarModel.getYear(index);
					font.pixelSize: appSettings.extraLargeFontSize
					font.bold: true
				}
			}

			DayOfWeekRow {
				id: weekTitles
				locale: monthGrid.locale
				anchors.top: monthYearTitle.bottom
				height: calendar.cellSize
				width: parent.width

				delegate: Text {
					text: model.shortName
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
					color: appSettings.fontColor
					font.bold: true
					font.pixelSize: appSettings.fontSize
				}
			}

			MonthGrid {
				id: monthGrid
				locale: Qt.locale(appSettings.appLocale)
				month: calendarModel.getMonth(index)
				year: calendarModel.getYear(index)
				spacing: 0
				anchors.top: weekTitles.bottom
				width: parent.width
				height: calendar.cellSize * 8

				property var selectedDay: null

				delegate: Rectangle {
					id: dayEntry
					height: calendar.cellSize
					width: calendar.cellSize
					radius: height * 0.5
					border.color: "green"
					border.width: bDayIsFinished ? 2 : 0
					opacity: !highlighted ? 1 : 0.5

					readonly property bool todayDate: model.month === _today.getMonth() && model.day === _today.getDate()
					property bool bIsTrainingDay: false
					property bool bDayIsFinished: false
					property bool highlighted: false

					function highlightDay(highlighted: bool): void {
						if (highlighted)
							animExpand.start();
						else
							animShrink.start();
					}

					function decorateRect(month: int, day: int, dayFinished: bool): void {
						if (month === monthGrid.month) {
							if (calendarModel.isTrainingDay(month, day)) {
								color = appSettings.listEntryColor2;
								bIsTrainingDay = true;
								bDayIsFinished = dayFinished;
								return;
							}
						}
						color = "transparent";
					}

					Connections {
						target: calendarModel
						function onDayIsFinishedChanged(date: Date, bFinished: bool) : void {
							decorateRect(date.getMonth(), date.getDate(), bFinished);
						}
					}

					Component.onCompleted: decorateRect(model.month+1, model.day, calendarModel.isDayFinished(model.month+1, model.day-1));

					Text {
						anchors.centerIn: parent
						text: monthGrid.month === model.month ? bIsTrainingDay ? model.day + "-" + calendarModel.getSplitLetter(model.month+1, model.day-1) : model.day : ""
						color: todayDate ? "red" : appSettings.fontColor
						font.bold: true
						font.pixelSize: appSettings.fontSize
					}

					SequentialAnimation { // Expand the button
						id: animExpand
						alwaysRunToEnd: true

						PropertyAnimation {
							target: dayEntry
							property: "scale"
							to: 1.4
							duration: 200
							easing.type: Easing.InOutCubic
						}
					}
					SequentialAnimation { // Shrink back to normal
						id: animShrink
						alwaysRunToEnd: true

						PropertyAnimation {
							target: dayEntry
							property: "scale"
							to: 1.0
							duration: 200
							easing.type: Easing.InOutCubic
						}
					}

					MouseArea {
						anchors.fill: parent
						hoverEnabled: true
						enabled: dayEntry.bIsTrainingDay

						onClicked: {
							if (monthGrid.selectedDay)
								monthGrid.selectedDay.highlightDay(false);
							dayEntry.highlightDay(true);
							monthGrid.selectedDay = dayEntry;
							selectDay(model.year, model.month, model.day);
						}

						onEntered: dayEntry.highlighted = true;
						onExited: dayEntry.highlighted = false;
					}
				} //delegate: Rectangle
			} //MonthGrid
		} //delegate: Rectangle
	} //ListView

	footer: TPToolBar {
		height: footerHeight*2.3

		TPLabel {
			id: lblInfo
			text: calendarManager.dayInfo(calendar.currentYear, calendar.currentMonth+1, calendar.currentDay-1)
			wrapMode: Text.WordWrap
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
			width: parent.width
			height: parent.height*0.25

			anchors {
				top: parent.top
				topMargin: 10
				horizontalCenter: parent.horizontalCenter
			}
		}

		TPComboBox {
			id: cboSplitLetter
			model: AppGlobals.splitModel
			currentIndex: indexOfValue(calendarManager.selectedSplitLetter);
			width: parent.width*0.2

			onActivated: (index) => optChangeOnlyThisDay.enabled = optChangeAfterThisDay.enabled = calendarManager.selectedSplitLetter !== valueAt(index);

			anchors {
				top: optChangeOnlyThisDay.bottom
				topMargin: -height/2
				left: parent.left
				leftMargin: 5
			}
		} //TPComboBox

		TPRadioButton {
			id: optChangeOnlyThisDay
			text: qsTr("Change only this day")

			anchors {
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: -height/2
				left: cboSplitLetter.right
				leftMargin: 10
				right: parent.right
			}
		}
		TPRadioButton {
			id: optChangeAfterThisDay
			text: qsTr("Adjust calendar from this day on")
			multiLine: true

			anchors {
				top: optChangeOnlyThisDay.bottom
				topMargin: 5
				left: cboSplitLetter.right
				leftMargin: 10
				right: parent.right
			}
		}

		TPButton {
			id: btnChangeCalendar
			text: qsTr("Change Calendar")
			imageSource: "edit-calendar.png"
			enabled: optChangeOnlyThisDay.checked || optChangeAfterThisDay.checked

			onClicked: {
				calendarManager.changeCalendar(optChangeAfterThisDay.checked, cboSplitLetter.currentValue);
				optChangeOnlyThisDay.checked = optChangeAfterThisDay.checked = false;
				optChangeOnlyThisDay.enabled = optChangeAfterThisDay.enabled = false;
			}

			anchors {
				left: parent.left
				leftMargin: 5
				bottom: parent.bottom
				bottomMargin: 5
			}
		}

		TPButton {
			id: btnViewWorkout
			text: qsTr("Workout")
			imageSource: "workout.png"

			anchors {
				right: parent.right
				rightMargin: 5
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: calendarManager.getTrainingDayPage(calendar.dayInfoDate);
		}
	} // footer: ToolBar

	Component.onCompleted: mesoCalendarPage.StackView.activating.connect(pageActivation);

	function pageActivation(): void {
		_today = new Date();
		if (!bAlreadyLoaded) {
			calendarModel.calendarChanged.connect(reloadModel);
			selectDay(_today.getFullYear(), _today.getMonth(), _today.getDate());
			calendar.positionViewAtIndex(calendarModel.getIndex(_today), ListView.Center);
			bAlreadyLoaded = true;
		}
	}

	function reloadModel(): void {
		calendar.model = null;
		calendar.model = calendarModel;
	}

	//Javascript date values differ from QDate's and TP's and Qt's
	//Month: JS 0-11 TP: 1-12 Qt:1-12
	//Date: JS 1-31 TP:0-30 Qt: 1-31
	function selectDay(year, month, day): void {
		calendar.currentDay = day;
		calendar.currentMonth = month;
		calendar.currentYear = year;
		calendar.dayInfoDate = new Date(year, month, day);
		optChangeOnlyThisDay.checked = optChangeAfterThisDay.checked = false;
		optChangeOnlyThisDay.enabled = optChangeAfterThisDay.enabled = false;
	}
} //Page
