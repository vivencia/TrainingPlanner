import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vivenciasoftware.qmlcomponents

Page {
	id: mesoContentPage
	required property int mesoId
	required property string mesoName
	required property date mesoStartDate
	required property date mesoEndDate
	required property string mesoSplit
	required property int idxModel
	required property bool bVisualLoad

	property var calendarModel: null

	property string splitLetter
	property string trainingDay
	property string splitContent
	property bool bReloadDatabase;
	property bool bCanViewDay
	property bool bCalendarInSyncWithMeso

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}

	header: ToolBar {
		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		ColumnLayout {
			anchors.fill: parent

			Label {
				id: lbl1
				text: mesoName
				font.bold: true
				color: "white"
				font.pixelSize: AppSettings.fontSize
				Layout.alignment: Qt.AlignCenter
				Layout.topMargin: 5
			}
			Label {
				id: lbl2
				color: "white"
				wrapMode: Text.WordWrap
				text: qsTr("from  <b>") + runCmd.formatDate(mesoStartDate) + qsTr("</b>  through  <b>") + runCmd.formatDate(mesoEndDate) + "</b>"
				font.pixelSize: AppSettings.fontSizeLists
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
		property var monthsNames: [qsTr("January"), qsTr("February"), qsTr("March"), qsTr("April"),
									qsTr("May"), qsTr("June"), qsTr("July"), qsTr("August"),
									qsTr("September"), qsTr("October"), qsTr("November"), qsTr("December")]

		delegate: Rectangle {
			height: calendar.cellSize * 10.5
			width: calendar.width - 20
			color: primaryDarkColor
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
					font.pixelSize: AppSettings.titleFontSizePixelSize
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
					font.pixelSize: AppSettings.fontSize
				}
			}

			MonthGrid {
				id: monthGrid
				locale: Qt.locale(AppSettings.appLocale)
				month: calendarModel.getMonth(index)
				year: calendarModel.getYear(index)
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
					readonly property bool todayDate: model.year === todayFull.getFullYear() && model.month === todayFull.getMonth() && model.day === todayFull.getDate()
					property bool bIsTrainingDay: false

					Component.onCompleted: {
						var colorValue = "transparent";
						if ( highlighted )
							return Material.primary;
						else {
							//if ( monthGrid.year === model.year) {
								if ( monthGrid.month === model.month ) {
									if (calendarModel.isTrainingDay(model.month+1, model.day-1)) {
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
						text: monthGrid.month === model.month ? calendarModel.isTrainingDay(model.month+1, model.day-1) ? model.day + "-" + calendarModel.getSplit(model.month+1, model.day-1) : model.day : ""
						scale: highlighted ? 1.4 : 1
						Behavior on scale { NumberAnimation { duration: 150 } }
						visible: parent.enabled
						color: todayDate ? "red" : "white"
						font.bold: true
						font.pixelSize: AppSettings.fontSize
					}

					MouseArea {
						anchors.fill: parent
						onClicked: {
							if (model.year <= mesoStartDate.getFullYear()) {
								if (model.month <= mesoStartDate.getMonth()) {
									if (model.month === mesoStartDate.getMonth())
										btnShowDayInfo.enabled = model.day >= mesoStartDate.getDate();
									else
										btnShowDayInfo.enabled = false;
								}
							}
							if (model.year >= mesoEndDate.getFullYear()) {
								if (model.month >= mesoEndDate.getMonth()) {
									if (model.month === mesoEndDate.getMonth())
										btnShowDayInfo.enabled = model.day <= mesoEndDate.getDate();
									else
										btnShowDayInfo.enabled = false;
								}
							}
							if (btnShowDayInfo.enabled) {
								splitLetter = calendarModel.getSplit(model.month+1, model.day-1);
								trainingDay = calendarModel.getTrainingDay(model.month+1, model.day-1);
								getDivisionContent(splitLetter);
							}
							calendar.currentDay = model.day;
							calendar.currentMonth = model.month;
							calendar.currentYear = model.year;
							calendar.dayInfoDate = new Date(model.year, model.month, model.day);
						}
					}

				} //delegate: Rectangle
			} //MonthGrid
		} //delegate: Rectangle
	} //ListView

	function setModel(model) {
		if (model.count === 0) {
			model.createModel(mesoId, mesoStartDate, mesoEndDate, mesoSplit);
			appDB.pass_object(model);
			appDB.createMesoCalendar();
		}
		calendarModel = model;
		calendar.model = model;
	}

	footer: ToolBar {
		width: parent.width
		height: 55
		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		RowLayout {
			anchors.fill: parent

			Label {
				text: btnShowDayInfo.enabled ? qsTr("Trainning day <b>#" + trainingDay + "</b> Division: <b>" + splitLetter + "</b> - <b>") + splitContent + "</b>" :
						qsTr("Day is not part of the current mesocycle")
				color: "white"
				wrapMode: Text.WordWrap
				font.pixelSize: AppSettings.fontSizeLists
				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
				Layout.maximumWidth: parent.width / 2
				Layout.leftMargin: 5
			}

			ButtonFlat {
				id: btnShowDayInfo
				text: qsTr("View Day")
				imageSource: "qrc:/images/"+lightIconFolder+"day-info.png"
				textUnderIcon: true
				Layout.alignment:  Qt.AlignRight | Qt.AlignVCenter
				Layout.rightMargin: 5

				onClicked: {
					function pushOntoStackView(object) {
						appDB.getQmlObject.disconnect(pushOntoStackView);
						object.tDay = trainingDay;
						object.splitLetter = splitLetter;
						appStackView.push(object, StackView.DontLoad);
					}

					appDB.getQmlObject.connect(pushOntoStackView);
					appDB.getTrainingDay(idxModel, calendar.dayInfoDate, appStackView);
				}
			}
		} // RowLayout

		//Component.onCompleted: {
		//	mesoContentPage.StackView.activating.connect(pageActivation);
		//}
	} // footer: ToolBar

	function getDivisionContent(splitletter) {
		switch (splitletter) {
			case 'A': splitContent = mesoSplitModel.get(idxModel, 2); break;
			case 'B': splitContent = mesoSplitModel.get(idxModel, 3); break;
			case 'C': splitContent = mesoSplitModel.get(idxModel, 4); break;
			case 'D': splitContent = mesoSplitModel.get(idxModel, 5); break;
			case 'E': splitContent = mesoSplitModel.get(idxModel, 6); break;
			case 'F': splitContent = mesoSplitModel.get(idxModel, 7); break;
		}
	}
} //Page
