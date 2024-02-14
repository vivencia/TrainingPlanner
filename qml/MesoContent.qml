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
					//text: calendar.monthsNames[calendarModel.getMonth(index)] + " " + calendarModel.getYear(index);
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
					readonly property bool highlighted: enabled && model.day === calendar.currentDay && model.month === calendar.currentMonth
					readonly property bool enabled: model.month === monthGrid.month
					readonly property bool todayDate: model.year === todayFull.getFullYear() && model.month === todayFull.getMonth() && model.day === todayFull.getDate()
					property bool bIsTrainingDay: false

					Component.onCompleted: {
						var colorValue = "transparent"; //Material.white is undefined
						if ( enabled && highlighted )
							return Material.primary;
						else {
							if ( monthGrid.year === model.year) {
								if ( monthGrid.month === model.month ) {
									//if (calendarModel.isTrainingDay(model.day-1)) {
									//	colorValue =  "steelblue";
									//	bIsTrainingDay = true;
									//}
								}
							}
							bIsTrainingDay = false;
						}
						color = colorValue
					}

					Text {
						anchors.centerIn: parent
						text: monthGrid.month === model.month ? calendarModel.isTrainingDay(model.month, model.day-1) ? model.day + "-" + calendarModel.getSplit(model.month, model.day-1) : model.day : model.day
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
								splitLetter = calendarModel.daySplitLetter(model.day-1);
								trainingDay = calendarModel.getTrainingDay(model.day-1);
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
					for (var i = 0; i < trainingDayInfoPages.length; ++i) {
						if (trainingDayInfoPages[i].date === calendar.dayInfoDate.getTime()) {
							appStackView.push(trainingDayInfoPages[i].Object, StackView.DontLoad);
							return;
						}
					}

					function generateObject() {
						var component = Qt.createComponent("TrainingDayInfo.qml", Qt.Asynchronous);

						function finishCreation() {
							var trainingDayInfoPage = component.createObject(mainwindow, {mainDate: calendar.dayInfoDate,
								tDay: trainingDay, splitLetter: splitLetter, mesoName: mesoName, mesoId: mesoId,
								bAlreadyLoaded:false
							});
							trainingDayInfoPage.mesoCalendarChanged.connect(databaseChanged);

							//Maximum of 3 pages loaded on memory. The latest page replace the earliest
							if (trainingDayInfoPages.length === 3) {
								trainingDayInfoPages[0].Object.destroy();
								trainingDayInfoPages[0] = trainingDayInfoPages[1];
								trainingDayInfoPages[1] = trainingDayInfoPages[2];
								trainingDayInfoPages.pop();
							}

							trainingDayInfoPages.push({ "date":calendar.dayInfoDate.getTime(), "Object" : trainingDayInfoPage });
							appStackView.push(trainingDayInfoPage, StackView.DontLoad);
						}

						if (component.status === Component.Ready)
							finishCreation();
						else
							component.statusChanged.connect(finishCreation);
					}

					generateObject();
				}
			}
		} // RowLayout

		//Component.onCompleted: {
		//	mesoContentPage.StackView.activating.connect(pageActivation);
		//}
	} // footer: ToolBar

	function getDivisionContent(splitletter) {
		let result = Database.getDivisionForMeso(mesoId);

		if (result) {
			switch (splitletter) {
				case 'A': splitContent = result[0].splitA; break;
				case 'B': splitContent = result[0].splitB; break;
				case 'C': splitContent = result[0].splitC; break;
				case 'D': splitContent = result[0].splitD; break;
				case 'E': splitContent = result[0].splitE; break;
				case 'F': splitContent = result[0].splitF; break;
			}
		}
	}
} //Page
