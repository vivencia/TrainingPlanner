import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vivenciasoftware.qmlcomponents

Page {
	id: mesoContentPage
	objectName: "mesoCalendarPage"
	width: windowWidth
	height: windowHeight

	required property int mesoId
	required property int mesoIdx
	required property DBMesoCalendarModel mesoCalendarModel

	readonly property string mesoName: mesocyclesModel.get(mesoIdx, 1)
	property date _today

	property string splitLetter
	property string trainingDay
	property string splitContent
	property bool bCanViewDay
	property bool bAlreadyLoaded: false

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}

	header: ToolBar {
		background: Rectangle {
			color: AppSettings.primaryDarkColor
			opacity: 0.7
		}

		ColumnLayout {
			anchors.fill: parent

			Label {
				id: lbl1
				text: mesoName
				font.bold: true
				color: "white"
				font.pointSize: AppSettings.fontSize
				Layout.alignment: Qt.AlignCenter
				Layout.topMargin: 5
			}
			Label {
				id: lbl2
				color: "white"
				wrapMode: Text.WordWrap
				text: qsTr("from  <b>") + runCmd.formatDate(mesocyclesModel.getDate(mesoIdx, 2)) +
						qsTr("</b>  through  <b>") + runCmd.formatDate(mesocyclesModel.getDate(mesoIdx, 3)) + "</b>"
				font.pointSize: AppSettings.fontSizeLists
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: parent.width - 10
				Layout.leftMargin: 5
				Layout.bottomMargin: 2
			}
		}
	}

	ListView {
		id: calendar

		//property date startDate
		property double mm: Screen.pixelDensity
		property double cellSize: mm * 7
		property int fontSizePx: calendar.cellSize * 0.32

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
			width: calendar.width - 20
			color: AppSettings.primaryDarkColor
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
					font.pointSize: AppSettings.fontSizeTitle
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
					color: "white"
					font.bold: true
					font.pointSize: AppSettings.fontSize
				}
			}

			MonthGrid {
				id: monthGrid
				locale: Qt.locale(AppSettings.appLocale)
				month: mesoCalendarModel.getMonth(index)
				year: mesoCalendarModel.getYear(index)
				spacing: 0
				anchors.top: weekTitles.bottom
				width: parent.width
				height: calendar.cellSize * 8

				delegate: Rectangle {
					id: dayEntry
					height: calendar.cellSize
					width: calendar.cellSize
					radius: height * 0.5
					readonly property bool highlighted: model.day === calendar.currentDay && model.month === calendar.currentMonth
					readonly property bool todayDate: model.year === _today.getFullYear() && model.month === _today.getMonth() && model.day === _today.getDate()
					property bool bIsTrainingDay: false

					Component.onCompleted: {
						var colorValue = "transparent";
						if ( highlighted )
							return Material.primary;
						else {
							//if ( monthGrid.year === model.year) {
								if ( monthGrid.month === model.month ) {
									if (mesoCalendarModel.isTrainingDay(model.month+1, model.day-1)) {
										colorValue =  listEntryColor2;
										bIsTrainingDay = true;
									}
								}
							//}
							bIsTrainingDay = false;
						}
						color = colorValue
					}

					Text {
						anchors.centerIn: parent
						text: monthGrid.month === model.month ? mesoCalendarModel.isTrainingDay(model.month+1, model.day-1) ? model.day + "-" + mesoCalendarModel.getSplitLetter(model.month+1, model.day-1) : model.day : ""
						scale: highlighted ? 1.4 : 1
						Behavior on scale { NumberAnimation { duration: 150 } }
						visible: parent.enabled
						color: todayDate ? "red" : "white"
						font.bold: true
						font.pointSize: AppSettings.fontSize
					}

					MouseArea {
						anchors.fill: parent
						onClicked: {
							selectDay(model.year, model.month, model.day);
						}
					}

				} //delegate: Rectangle
			} //MonthGrid
		} //delegate: Rectangle
	} //ListView

	footer: ToolBar {
		width: parent.width
		height: 55
		background: Rectangle {
			color: AppSettings.primaryDarkColor
			opacity: 0.7
		}

		RowLayout {
			anchors.fill: parent

			Label {
				text: btnShowDayInfo.enabled ? qsTr("Trainning day <b>#" + trainingDay + "</b> Division: <b>" + splitLetter + "</b> - <b>") + splitContent + "</b>" :
						qsTr("Selected day is not part of the current mesocycle")
				color: "white"
				wrapMode: Text.WordWrap
				font.pointSize: AppSettings.fontSizeLists
				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
				Layout.maximumWidth: parent.width / 2
				Layout.leftMargin: 5
			}

			TPButton {
				id: btnShowDayInfo
				text: qsTr("View Day")
				imageSource: "qrc:/images/"+lightIconFolder+"day-info.png"
				textUnderIcon: true
				Layout.alignment:  Qt.AlignRight | Qt.AlignVCenter
				Layout.rightMargin: 5
				enabled: false

				onClicked: {
					function pushTDayOntoStackView(page, id) {
						if (id === 70) {
							appDB.getPage.disconnect(pushTDayOntoStackView);
							appMainMenu.addShortCut( qsTr("Workout: ") + runCmd.formatDate(_today) , page);
						}
					}

					appDB.getPage.connect(pushTDayOntoStackView);
					appDB.getTrainingDay(calendar.dayInfoDate);
				}
			}
		} // RowLayout
	} // footer: ToolBar

	Component.onCompleted: {
		mesoContentPage.StackView.activating.connect(pageActivation);
	}

	function pageActivation() {
		_today = new Date();
		if (!bAlreadyLoaded) {
			calendar.model = mesoCalendarModel;
			mesoCalendarModel.calendarChanged.connect(reloadModel);
			selectDay(_today.getFullYear(), _today.getMonth(), _today.getDate());
			bAlreadyLoaded = true;
		}
		calendar.positionViewAtIndex(mesoCalendarModel.getIndex(_today), ListView.Center);
	}

	function reloadModel() {
		calendar.model = null;
		calendar.model = mesoCalendarModel;
	}

	//Javascript date values differ from QDate's and TP's.
	//Month: JS 0-11 TP: 1-12
	//Date: JS 1-31 TP:0-30
	function selectDay(year, month, day) {
		btnShowDayInfo.enabled = mesoCalendarModel.isTrainingDay(month+1, day-1);

		if (btnShowDayInfo.enabled) {
			splitLetter = mesoCalendarModel.getSplitLetter(month+1, day-1);
			trainingDay = mesoCalendarModel.getTrainingDay(month+1, day-1);
			getDivisionContent(splitLetter);
		}
		calendar.currentDay = day;
		calendar.currentMonth = month;
		calendar.currentYear = year;
		calendar.dayInfoDate = new Date(year, month, day);
	}

	function getDivisionContent(splitletter) {
		switch (splitletter) {
			case 'A': splitContent = mesoSplitModel.get(mesoIdx, 2); break;
			case 'B': splitContent = mesoSplitModel.get(mesoIdx, 3); break;
			case 'C': splitContent = mesoSplitModel.get(mesoIdx, 4); break;
			case 'D': splitContent = mesoSplitModel.get(mesoIdx, 5); break;
			case 'E': splitContent = mesoSplitModel.get(mesoIdx, 6); break;
			case 'F': splitContent = mesoSplitModel.get(mesoIdx, 7); break;
		}
	}
} //Page
