import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"
import "../TPWidgets"

TPPage {
	id: mesoCalendarPage
	objectName: "mesoCalendarPage"

	required property MesoManager mesoManager
	required property DBMesoCalendarModel mesoCalendarModel

	property date _today
	property bool bAlreadyLoaded: false

	header: ToolBar {
		height: 60

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		ColumnLayout {
			anchors.fill: parent
			spacing: 0

			TPLabel {
				id: lbl1
				text: mesoManager.name
				font: AppGlobals.titleFont
				Layout.maximumWidth: parent.width - 10
				Layout.minimumHeight: heightAvailable
				Layout.maximumHeight: heightAvailable
				Layout.alignment: Qt.AlignCenter
				Layout.topMargin: 5
			}
			TPLabel {
				id: lbl2
				text: qsTr("from  <b>") + mesoManager.startDate + qsTr("</b>  through  <b>") + mesoManager.endDate + "</b>"
				font: AppGlobals.titleFont
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: parent.width - 10
				Layout.minimumHeight: heightAvailable
				Layout.maximumHeight: heightAvailable
				Layout.leftMargin: 5
				Layout.topMargin: 5
				Layout.bottomMargin: 5
			}
		}
	}

	ListView {
		id: calendar

		//property date startDate
		readonly property double mm: Screen.pixelDensity
		readonly property double cellSize: mm * 7
		readonly property int fontSizePx: calendar.cellSize * 0.32

		anchors.fill: parent
		snapMode: ListView.SnapToItem
		spacing: 2
		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true//ScrollBar.AlwaysOn
		}

		property date dayInfoDate
		property int currentDay
		property int currentMonth
		property int currentYear
		readonly property var monthsNames: [qsTr("January"), qsTr("February"), qsTr("March"), qsTr("April"),
									qsTr("May"), qsTr("June"), qsTr("July"), qsTr("August"),
									qsTr("September"), qsTr("October"), qsTr("November"), qsTr("December")]

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
					text: calendar.monthsNames[mesoCalendarModel.getMonth(index)] + " " + mesoCalendarModel.getYear(index);
					font.pointSize: appSettings.fontSizeTitle
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
					font.pointSize: appSettings.fontSize
				}
			}

			MonthGrid {
				id: monthGrid
				locale: Qt.locale(appSettings.appLocale)
				month: mesoCalendarModel.getMonth(index)
				year: mesoCalendarModel.getYear(index)
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
					border.color: "red"
					border.width: bDayIsFinished ? 2 : 0
					opacity: !highlighted ? 1 : 0.5

					readonly property bool todayDate: model.month === _today.getMonth() && model.day === _today.getDate()
					property bool bIsTrainingDay: false
					property bool bDayIsFinished: false
					property bool highlighted: false
					property string colorValue

					Component.onCompleted: {
						var colorValue = "transparent";
						if ( monthGrid.month === model.month ) {
							if (mesoCalendarModel.isTrainingDay(model.month+1, model.day-1)) {
								colorValue =  appSettings.listEntryColor2;
								bIsTrainingDay = true;
								bDayIsFinished = mesoCalendarModel.isDayFinished(model.month+1, model.day-1);
							}
						}
						color = colorValue
					}

					Text {
						anchors.centerIn: parent
						text: monthGrid.month === model.month ? bIsTrainingDay ? model.day + "-" + mesoCalendarModel.getSplitLetter(model.month+1, model.day-1) : model.day : ""
						color: todayDate ? "red" : appSettings.fontColor
						font.bold: true
						font.pointSize: appSettings.fontSize
					}

					SequentialAnimation {
						id: animExpand
						alwaysRunToEnd: true
						// Expand the button
						PropertyAnimation {
							target: dayEntry
							property: "scale"
							to: 1.4
							duration: 200
							easing.type: Easing.InOutCubic
						}
					}
					SequentialAnimation {
						id: animShrink
						alwaysRunToEnd: true
						// Shrink back to normal
						PropertyAnimation {
							target: dayEntry
							property: "scale"
							to: 1.0
							duration: 200
							easing.type: Easing.InOutCubic
						}
					}

					function highlightDay(highlighted: bool) {
						if (highlighted)
							animExpand.start();
						else
							animShrink.start();
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

	footer: ToolBar {
		width: parent.width
		height: footerHeight

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		Label {
			id: lblInfo
			color: appSettings.fontColor
			width: parent.width - btnViewWorkout.width - 10
			wrapMode: Text.WordWrap
			font.pointSize: appSettings.fontSizeText
			font.bold: true
			anchors {
				left: parent.left
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}
		}

		TPButton {
			id: btnViewWorkout
			text: qsTr("Workout")
			imageSource: "workout.png"
			textUnderIcon: true
			rounded: false
			flat: false
			width: 60
			height: 55
			fixedSize: true

			anchors {
				right: parent.right
				rightMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: mesoManager.getTrainingDayPage(calendar.dayInfoDate);
		}
	} // footer: ToolBar

	Component.onCompleted: mesoCalendarPage.StackView.activating.connect(pageActivation);

	function pageActivation() {
		_today = new Date();
		if (!bAlreadyLoaded) {
			calendar.model = mesoCalendarModel;
			mesoCalendarModel.calendarChanged.connect(reloadModel);
			selectDay(_today.getFullYear(), _today.getMonth(), _today.getDate());
			calendar.positionViewAtIndex(mesoCalendarModel.getIndex(_today), ListView.Center);
			bAlreadyLoaded = true;
		}
	}

	function reloadModel() {
		calendar.model = null;
		calendar.model = mesoCalendarModel;
	}

	//Javascript date values differ from QDate's and TP's.
	//Month: JS 0-11 TP: 1-12
	//Date: JS 1-31 TP:0-30
	function selectDay(year, month, day) {
		calendar.currentDay = day;
		calendar.currentMonth = month;
		calendar.currentYear = year;
		calendar.dayInfoDate = new Date(year, month, day);
		lblInfo.text = mesoCalendarModel.getInfoLabelText(year, month+1, day-1);
	}
} //Page
