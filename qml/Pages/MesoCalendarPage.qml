import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"
import "../TPWidgets"

TPPage {
	id: mesoCalendarPage
	imageSource: ":/images/backgrounds/calendar_backimage.jpg"
	objectName: "mesoCalendarPage"

	required property CalendarManager calendarManager
	property DBCalendarModel calendarModel

	property date _today: new Date()
	property bool bAlreadyLoaded: false

	onPageActivated: {
		if (!bAlreadyLoaded && calendarModel !== null)
		{
			calendar.positionViewAtIndex(calendarModel.getIndexFromDate(_today), ListView.Contain);
			calendarManager.selectedDate = _today;
			bAlreadyLoaded = true;
		}
	}

	Connections {
		target: calendarManager
		function onSelectedDateChanged() : void {
			lblInfo.text = calendarManager.dayInfo();
			cboSplitLetter.currentIndex = cboSplitLetter.indexOfValue(calendarManager.selectedSplitLetter);
		}
	}

	header: TPToolBar {
		height: appSettings.pageHeight * 0.1

		ColumnLayout {
			anchors.fill: parent
			spacing: 0

			TPLabel {
				id: lbl1
				text: calendarManager.nameLabel
				font: AppGlobals.extraLargeFont
				singleLine: true
				Layout.maximumWidth: parent.width*0.8
				Layout.alignment: Qt.AlignCenter
			}
			TPLabel {
				id: lbl2
				text: calendarManager.dateLabel
				font: AppGlobals.regularFont
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: parent.width - 10
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				Layout.bottomMargin: 5
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
			height: calendar.cellSize * 10.5
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
					width: calendar.cellSize * 0.8
					height: width
					radius: width * 0.5
					border.color: "green"
					border.width: dayIsFinished ? 2 : 0
					opacity: !highlighted ? 1 : 0.5
					color: appSettings.primaryLightColor

					readonly property date month_day: new Date(model.year, model.month, model.day);
					readonly property bool todayDate: month_day.getUTCFullYear() === _today.getUTCFullYear() &&
								month_day.getUTCMonth() === _today.getUTCMonth() && month_day.getUTCDate() === _today.getUTCDate()
					property bool dayIsFinished: calendarModel.completed(month_day)
					property bool highlighted: false

					function highlightDay(highlighted: bool): void {
						if (highlighted)
							animExpand.start();
						else
							animShrink.start();
					}

					Connections {
						enabled: calendarModel !== null
						target: calendarModel
						function onCompletedChanged(date: Date) : void {
							if (date === dayEntry.month_day)
								dayIsFinished = calendarModel.completed(date);
						}
					}

					Text {
						id: txtDay
						anchors.centerIn: parent
						text: calendarModel.dayText(dayEntry.month_day)
						color: !dayEntry.todayDate ? (calendarModel.isPartOfMeso(dayEntry.month_day) ? appSettings.fontColor : appSettings.disabledFontColor) : "red"
						font.bold: true
						font.pixelSize: appSettings.fontSize

						Connections {
							enabled: calendarModel !== null
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
		height: appSettings.pageHeight / 4

		TPLabel {
			id: lblInfo
			text: calendarManager.dayInfo()
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
			currentIndex: indexOfValue(calendarManager.selectedSplitLetter);
			width: parent.width * 0.2

			onActivated: (index) => optChangeOnlyThisDay.enabled = optChangeAfterThisDay.enabled =
															calendarManager.selectedSplitLetter !== valueAt(index);

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
				topMargin: 5
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
			autoSize: true

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
			autoSize: true

			anchors {
				right: parent.right
				rightMargin: 5
				bottom: parent.bottom
				bottomMargin: 5
			}

			onClicked: calendarManager.getWorkoutPage();
		}
	} // footer: ToolBar

	//Javascript month values differ from QDate's
	//JS 0-11 Qt:1-12
	function selectDay(year, month, day): void {
		calendarManager.selectedDate = new Date(year, month, day);
		optChangeOnlyThisDay.checked = optChangeAfterThisDay.checked = false;
		optChangeOnlyThisDay.enabled = optChangeAfterThisDay.enabled = false;
	}
} //Page
