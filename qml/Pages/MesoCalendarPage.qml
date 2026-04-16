pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPage {
	id: mesoCalendarPage
	imageSource: ":/images/backgrounds/backimage-calendar.jpg"
	backgroundOpacity: 0.6
	objectName: "mesoCalendarPage"

//public:
	required property CalendarManager calendarManager
	property DBCalendarModel calendarModel

//private:
	property date _today: new Date()

	onPageActivated: calendar.positionViewAtIndex(mesoCalendarPage.calendarModel.getIndexFromDate(
																	mesoCalendarPage.calendarModel.currentDate), ListView.Contain);

	header: TPToolBar {
		height: AppSettings.pageHeight * 0.1
		padding: 0
		spacing: 0

		ColumnLayout {
			anchors.fill: parent
			spacing: 0

			TPLabel {
				id: lbl1
				text: mesoCalendarPage.calendarManager.nameLabel
				font: AppGlobals.extraLargeFont
				horizontalAlignment: Text.AlignHCenter
				Layout.maximumWidth: parent.width - 10
				Layout.maximumHeight: parent.height * 0.65
			}
			TPLabel {
				id: lbl2
				text: mesoCalendarPage.calendarManager.dateLabel
				font: AppGlobals.regularFont
				horizontalAlignment: Text.AlignHCenter
				Layout.maximumWidth: parent.width - 10
				Layout.maximumHeight: parent.height * 0.3
			}
		}
	}

	ListView {
		id: calendar
		model: mesoCalendarPage.calendarModel
		reuseItems: true
		snapMode: ListView.SnapToItem
		spacing: cellSize
		anchors.fill: parent

		readonly property double _size_factor: 7
		readonly property double mm: Screen.pixelDensity
		readonly property double cellSize: mm * 7
		readonly property int fontSizePx: cellSize * (_size_factor / 21) //0.32

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
		}

		delegate: Rectangle {
			id: delegate
			height: calendar.cellSize * 11
			width: calendar.width - 10
			color: AppSettings.primaryDarkColor
			opacity: 0.7

			required property int index

			Rectangle {
				id: monthYearTitle
				anchors {
					top: parent.top
				}
				height: calendar.cellSize * 1.5
				width: parent.width

				Text {
					anchors.centerIn: parent
					text: AppUtils.monthName(mesoCalendarPage.calendarModel.month(delegate.index)) + " " +
																				mesoCalendarPage.calendarModel.year(delegate.index);
					font.pixelSize: calendar.fontSizePx * 1.5
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
					text: shortName
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
					color: AppSettings.fontColor
					font.bold: true
					font.pixelSize: calendar.fontSizePx* 1.3

					required property string shortName
				}
			}

			MonthGrid {
				id: monthGrid
				locale: Qt.locale(AppSettings.userLocale)
				month: mesoCalendarPage.calendarModel.month(delegate.index)
				year: mesoCalendarPage.calendarModel.year(delegate.index)
				spacing: 2
				anchors.top: weekTitles.bottom
				width: calendar.cellSize * 12
				height: calendar.cellSize * 10

				delegate: TPCalendarEntry {
					entryDay: day
					entryMonth: month
					entryYear: year
					cellSize: calendar.cellSize
					today: mesoCalendarPage._today
					parentMonth: monthGrid.month
					tpCalendarModel: mesoCalendarPage.calendarModel
					selectedDate: mesoCalendarPage.calendarModel.currentDate

					required property int day
					required property int month
					required property int year

					onDateSelected: (day, month, year, is_workout) => {
						mesoCalendarPage.calendarModel.currentDate = new Date(year, month, day);
						optChangeOnlyThisDay.checked = optChangeAfterThisDay.checked = false;
						optChangeOnlyThisDay.enabled = optChangeAfterThisDay.enabled = false;
						btnViewWorkout.enabled = is_workout;
						lblInfo.text = mesoCalendarPage.calendarManager.dayInfo();
						cboSplitLetter.currentIndex = mesoCalendarPage.calendarModel.splitLetterToIndex();
					}
				} //delegate
			} //MonthGrid
		} //delegate: Rectangle
	} //ListView

	footer: TPToolBar {
		height: AppSettings.pageHeight / 4
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
																		mesoCalendarPage.calendarModel.splitLetter !== valueAt(index);

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
			height: AppSettings.itemExtraLargeHeight

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
				mesoCalendarPage.calendarManager.changeSplitLetter(cboSplitLetter.currentValue, optChangeAfterThisDay.checked);
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

			onClicked: mesoCalendarPage.calendarManager.getWorkoutPage();
		}
	} // footer: ToolBar
} //Page
