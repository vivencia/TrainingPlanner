import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"
import "../TPWidgets"

TPPage {
	id: mesoCalendarPage
	imageSource: ":/images/backgrounds/backimage-calendar.jpg"
	backgroundOpacity: 0.6
	objectName: "mesoCalendarPage"

	required property CalendarManager calendarManager
	property DBCalendarModel calendarModel

	property date _today: new Date()

	onPageActivated: calendar.positionViewAtIndex(calendarModel.getIndexFromDate(calendarModel.currentDate), ListView.Contain);

	header: TPToolBar {
		height: appSettings.pageHeight * 0.1
		padding: 0
		spacing: 0

		ColumnLayout {
			anchors.fill: parent
			spacing: 0

			TPLabel {
				id: lbl1
				text: calendarManager.nameLabel
				font: AppGlobals.extraLargeFont
				horizontalAlignment: Text.AlignHCenter
				Layout.maximumWidth: parent.width - 10
				Layout.maximumHeight: parent.height * 0.65
			}
			TPLabel {
				id: lbl2
				text: calendarManager.dateLabel
				font: AppGlobals.regularFont
				horizontalAlignment: Text.AlignHCenter
				Layout.maximumWidth: parent.width - 10
				Layout.maximumHeight: parent.height * 0.3
			}
		}
	}

	ListView {
		id: calendar
		model: calendarModel
		reuseItems: true
		snapMode: ListView.SnapToItem
		spacing: 2
		anchors.fill: parent

		readonly property double mm: Screen.pixelDensity
		readonly property double cellSize: mm * 7

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true//ScrollBar.AlwaysOn
		}

		delegate: Rectangle {
			height: calendar.cellSize * 11
			width: calendar.width - 10
			color: appSettings.primaryDarkColor
			opacity: 0.7

			Rectangle {
				id: monthYearTitle
				anchors {
					top: parent.top
				}
				height: calendar.cellSize * 1.5
				width: parent.width

				Text {
					anchors.centerIn: parent
					text: appUtils.monthName(calendarModel.month(index)) + " " + calendarModel.year(index);
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
				locale: Qt.locale(appSettings.userLocale)
				month: calendarModel.month(index)
				year: calendarModel.year(index)
				spacing: 2
				anchors.top: weekTitles.bottom
				width: parent.width
				height: calendar.cellSize * 8

				property Rectangle selectedDay: null

				delegate: Rectangle {
					id: dayEntry
					radius: width * 0.5
					border.color: "green"
					border.width: workoutFinished ? 2 : 0
					opacity: workoutDay ? 1 : mesoDay ?  0.7 : 0.4
					color: visibleDay ? appSettings.primaryLightColor : "transparent"

					function dateSelected(): void {
						highlightDay(true);
						monthGrid.selectedDay = this;
						calendarModel.currentDate = month_day;
						optChangeOnlyThisDay.checked = optChangeAfterThisDay.checked = false;
						optChangeOnlyThisDay.enabled = optChangeAfterThisDay.enabled = false;
						btnViewWorkout.enabled = workoutDay;
						lblInfo.text = calendarManager.dayInfo();
						cboSplitLetter.currentIndex = calendarModel.splitLetterToIndex();
					}

					readonly property date month_day: new Date(model.year, model.month, model.day);
					readonly property bool todayDate: month_day.getUTCFullYear() === _today.getUTCFullYear() &&
							month_day.getUTCMonth() === _today.getUTCMonth() && month_day.getUTCDate() === _today.getUTCDate()
					readonly property bool visibleDay: model.month === monthGrid.month
					readonly property bool mesoDay: calendarModel.isPartOfMeso(month_day)
					readonly property bool workoutDay: calendarModel.isWorkoutDay(month_day)
					readonly property bool workoutFinished: calendarModel.completed_by_date(month_day)

					function highlightDay(highlighted: bool): void {
						if (highlighted)
							animExpand.start();
						else
							animShrink.start();
					}

					Component.onCompleted: {
						if (todayDate)
							dateSelected();
					}

					Connections {
						enabled: calendarModel !== null
						target: calendarModel
						function onCompletedChanged(date: Date) : void {
							if (date === dayEntry.month_day)
								workoutFinished = calendarModel.completed(date);
						}
					}

					TPLabel {
						id: txtDay
						anchors.centerIn: parent
						text: calendarModel.dayEntryLabel(dayEntry.month_day)
						font: AppGlobals.smallFont
						visible: dayEntry.visibleDay
						color: !dayEntry.todayDate ? (mesoDay ? appSettings.fontColor : appSettings.disabledFontColor) : "red"

						Connections {
							target: calendarModel
							function onSplitLetterChanged(date: Date) : void {
								txtDay.text = calendarModel.dayText(date);
							}
						}
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

						onClicked: {
							if (monthGrid.selectedDay)
								monthGrid.selectedDay.highlightDay(false);
							dayEntry.dateSelected();
						}
					}
				} //delegate: Rectangle
			} //MonthGrid
		} //delegate: Rectangle
	} //ListView

	footer: TPToolBar {
		height: appSettings.pageHeight / 4
		padding: 0
		spacing: 0

		TPLabel {
			id: lblInfo
			wrapMode: Text.WordWrap
			horizontalAlignment: Text.AlignHCenter
			width: parent.width
			height: parent.height / 4

			anchors {
				top: parent.top
				topMargin: 5
				horizontalCenter: parent.horizontalCenter
			}
		}

		TPComboBox {
			id: cboSplitLetter
			model: AppGlobals.splitModel
			width: parent.width * 0.2

			onActivated: (index) => optChangeOnlyThisDay.enabled = optChangeAfterThisDay.enabled =
																				calendarModel.splitLetter !== valueAt(index);

			anchors {
				top: optChangeOnlyThisDay.bottom
				topMargin: -height / 2
				left: parent.left
				leftMargin: 5
			}
		} //TPComboBox

		TPRadioButtonOrCheckBox {
			id: optChangeOnlyThisDay
			text: qsTr("Change only this day")

			anchors {
				top: lblInfo.bottom
				left: cboSplitLetter.right
				leftMargin: 10
				right: parent.right
			}
		}
		TPRadioButtonOrCheckBox {
			id: optChangeAfterThisDay
			text: qsTr("Adjust calendar from this day on")
			multiLine: true
			height: appSettings.itemExtraLargeHeight

			anchors {
				top: optChangeOnlyThisDay.bottom
				topMargin: -5
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
			width: parent.width * 0.65

			onClicked: {
				calendarManager.changeSplitLetter(cboSplitLetter.currentValue, optChangeAfterThisDay.checked);
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
			width: parent.width * 0.3

			anchors {
				right: parent.right
				rightMargin: 5
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: calendarManager.getWorkoutPage();
		}
	} // footer: ToolBar
} //Page
