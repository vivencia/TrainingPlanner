import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "jsfunctions.js" as JSF

Page {
	id: mesoContentPage
	required property int mesoId
	required property string mesoName
	required property date mesoStartDate
	required property date mesoEndDate
	required property string mesoSplit
	required property bool bVisualLoad

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

	function refactoryDatabase(newStartDate, newEndDate, newSplit, bPreserveOldInfo, bPreserveOldInfoUntilToday) {
		if (bCalendarInSyncWithMeso) return; //See comment under readDatabase()

		console.log(newStartDate.toDateString() + "    " + mesoStartDate.toDateString());
		console.log(newEndDate.toDateString() + "    " + mesoEndDate.toDateString());
		console.log(newSplit + "    " + mesoSplit);

		//First, populate the Array as if starting a new calendar
		let old_monthsmodel = [];
		let monthsmodel = getMesoMonths(newStartDate, newEndDate, newSplit);
		if (bPreserveOldInfo) { //preserve old calendar info (splitLetter and trainingDay)
			old_monthsmodel = getMesoMonths(mesoStartDate, mesoEndDate, mesoSplit);
			var i = 0, x = 0, iz = 0, xz = 0;
			var startyear, startmonth, startday;
			var bFound = false;
			var bInitialPaddingAccounted = false;

			if (newStartDate <= mesoStartDate) { //If newStartDate and mesoStartDate are the same, the = sign could be in either of these conditionals
				//New meso will start earlier than the old one. Navigate through current model until we reach the old one start date
				startyear = mesoStartDate.getFullYear();
				startmonth = mesoStartDate.getMonth();
				startday = mesoStartDate.getDate();
			}
			else if (newStartDate > mesoStartDate) {
				//New meso will start later than the old one. Cannot copy old date into days that do not exis anymore. Navigate
				//through the old model until we reach the start date of the new model
				startyear = newStartDate.getFullYear();
				startmonth = newStartDate.getMonth();
				startday = newStartDate.getDate();
			}

			for(i = 0; i < monthsmodel.length && bFound === false; i++) {
				if (monthsmodel[i].yearNbr === startyear) {
					if (monthsmodel[i].monthNbr === startmonth) {
						while ( iz < monthsmodel[i].daySplit.length ) {
							if (monthsmodel[i].daySplit[iz].dayNbr === startday) {
								bFound = true;
								break;
							}
							iz++;
						}
					}
				}
			}
			if (bFound) {
				--i; //the increment happens before the conditional checking after the first iteration
				bFound = false;
			}

			//Navigate through the old info until we find the right start date. The model for the month might include padding information to fill
			//the whole month (before startday and after end day) not only the meso included dates
			for(x = 0; x < old_monthsmodel.length && bFound === false; x++) {
				if (old_monthsmodel[x].yearNbr === startyear) {
					if (old_monthsmodel[x].monthNbr === startmonth) {
						while ( xz < old_monthsmodel[x].daySplit.length ) {
							if (monthsmodel[x].daySplit[xz].dayNbr === startday) {
								bFound = true;
								break;
							}
							xz++;
						}
					}
				}
			}
			if (bFound) --x;

			var enddate;
			if (bPreserveOldInfoUntilToday) //preserve old info only until the day before the updating of the meso
				enddate = JSF.getPreviousDate(today);
			else
				enddate = mesoEndDate; //preserve all the old info.
			if (enddate > newEndDate) // The new meso might be shorter than the old and even end before today
				enddate = newEndDate;

			const endyear = enddate.getFullYear();
			const endmonth = enddate.getMonth();
			const endday = enddate.getDate();

			while (i < monthsmodel.length) { // => Start copying old information into new. i is either 0 or found in the iterations above. z too
				while ( iz < monthsmodel[i].daySplit.length ) {
					//console.log("Prev info: " + monthsmodel[i].yearNbr + "/" + monthsmodel[i].monthNbr + "/" + monthsmodel[i].daySplit[iz].dayNbr);
					//console.log(monthsmodel[i].daySplit[iz].daySplitLetter);
					monthsmodel[i].daySplit[iz].daySplitLetter = old_monthsmodel[x].daySplit[xz].daySplitLetter;
					monthsmodel[i].daySplit[iz].isTrainingDay = old_monthsmodel[x].daySplit[xz].isTrainingDay;
					monthsmodel[i].daySplit[iz].trainingDayNumber = old_monthsmodel[x].daySplit[xz].trainingDayNumber;
					//console.log("New info: " + monthsmodel[i].yearNbr + "/" + monthsmodel[i].monthNbr + "/" + monthsmodel[i].daySplit[iz].dayNbr);
					//console.log(monthsmodel[i].daySplit[iz].daySplitLetter);
					xz++;
					if (xz >= old_monthsmodel[x].daySplit.length)
						break;
					iz++;
					if (monthsmodel[i].daySplit[iz].dayNbr > endday) {
						if (monthsmodel[i].monthNbr >= endmonth) {
							if (monthsmodel[i].yearNbr >= endyear) {
								x = old_monthsmodel.length; //just to force another break below and leave the loops
								break;
							}
						}
					}
				}
				x++;
				if (x >= old_monthsmodel.length) break;
				i++;
				iz = xz = 0;
			}
		}

		Database.deleteMesoCalendar(mesoId);
		//saveModelToDatabase(monthsmodel);
		dlgProgressIndicator.months_arr = monthsmodel;
		dlgProgressIndicator.meso_id = mesoId;
		dlgProgressIndicator.open();
		dlgProgressIndicator.init("Creating database. Please wait...", 0, 50);
		mesoMonthsModel.clear();
		for (let newmonth of monthsmodel)
			mesoMonthsModel.append(newmonth);
	}

	function convertCalendarToMonthsModel(calendar) {
		var year, month, day;
		var month2 = -1;
		var firstday = -1;
		var calDate;
		let monthsmodel = [];
		let splitmodel;

		for (var i = 0; i < calendar.length; i++) {
			calDate = new Date(calendar[i].mesoCalDate);
			//console.log(calDate.toString() + "  *****  " + calDate.getDate() + "/" + calDate.getMonth() );

			month = calDate.getMonth();
			if (month !== month2) {
				if (firstday !== -1) {
					year =  calDate.getFullYear();
					if (month2 === 11)
						year -= 1;
					monthsmodel.push({
									 "monthNbr": month2,
									 "yearNbr": year,
									 "firstDay": firstday,
									 "lastDay": day,
									 "daySplit": splitmodel
									 });
					firstday = -1;
				}
			}

			if (firstday === -1) {
				let newSplitModel = [];
				splitmodel = newSplitModel;
				firstday = calDate.getDate();
				day = firstday;
				month2 = month;
			}
			else {
				day = calDate.getDate();
			}

			splitmodel.push({
				"validMonth": month,
				"dayNbr": day,
				"daySplitLetter": calendar[i].mesoCalSplit,
				"isTrainingDay": calendar[i].mesoCalnDay !== 0 ? true : false,
				"trainingDayNumber": calendar[i].mesoCalnDay
			});
		}
		year =  calDate.getFullYear();
		if (month2 === 11)
			year -= 1;
			monthsmodel.push({
					 "monthNbr": month2,
					 "yearNbr": year,
					 "firstDay": firstday,
					 "lastDay": day,
					 "daySplit": splitmodel
		});
		return monthsmodel;
	}

	function getMesoMonths(startdate, enddate, strsplit) {
		var nMonths;
		const startyear = startdate.getFullYear();
		const startmonth = startdate.getMonth();
		const startday = startdate.getDate();
		const endmonth = enddate.getMonth();
		const endday = enddate.getDate();

		if (endmonth > startmonth)
			nMonths = endmonth - startmonth + 1;
		else {
			nMonths = (11 - startmonth) + endmonth + 1;
			if (endmonth === 0)
				nMonths++;
		}

		var trainingDayNumber = 1;
		let monthsmodel = [];
		var firstday, lastday;
		var splitrestidx = strsplit.indexOf("R", 0);
		var splitidx = 0;
		for (var i = 0, month = startmonth, year = startyear; i < nMonths; i++, month++)  {
			if ( month >= 12 ) {
				year++;
				month = 0;
			}
			if ( month === startmonth )
				firstday = startday;
			else
				firstday = 1;

			if ( month === endmonth )
				lastday = endday;
			else
				lastday = JSF.getMonthTotalDays(month,year);

			let splitmodel = [];
			var lastdayofmonth = JSF.getMonthTotalDays(month,year);

			for(var day = 1; day <= lastdayofmonth; day++ ) {
				if ( day >= firstday && day <= lastday) {
					//console.log(year + "/" + month + "/" + day);
					//console.log("daySplitLetter: " + strsplit.charAt(splitidx) + "  trainingDayNumber: " + ((splitidx !== splitrestidx) ? trainingDayNumber : 0));
					splitmodel.push({
						"validMonth": month,
						"dayNbr": day,
						"daySplitLetter": strsplit.charAt(splitidx),
						"isTrainingDay": (splitidx !== splitrestidx) ? true : false,
						"trainingDayNumber": (splitidx !== splitrestidx) ? trainingDayNumber : 0
					});

					splitidx++;
					if ( splitidx <= splitrestidx )
						trainingDayNumber++;
					else if ( splitidx > splitrestidx ) {
						splitrestidx = strsplit.indexOf("R", splitrestidx+1);
						if (splitrestidx === -1) {
							splitidx = 0;
							splitrestidx = strsplit.indexOf("R", 0);
						}
					}
				}
				else {
					splitmodel.push({
						"validMonth": -1,
						"dayNbr": day,
						"daySplitLetter": " ",
						"isTrainingDay": false,
						"trainingDayNumber": 0
					});
				}
			}

			monthsmodel.push({
				"monthNbr": month,
				"yearNbr": year,
				"firstDay": firstday,
				"lastDay": lastday,
				"daySplit": splitmodel
			});
		}

		/*for (let mm of monthsmodel) {
			for ( x = 0; x < mm.daySplit.length; x++ ) {
				console.log(mm.daySplit[x].dayNbr + "/" + mm.daySplit[x].validMonth + " - Training # " + mm.daySplit[x].trainingDayNumber + ": " + mm.daySplit[x].daySplitLetter + " is training day? " + mm.daySplit[x].isTrainingDay)
			}
		}*/
		return monthsmodel;
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
				text: qsTr("from  <b>") + JSF.formatDateToDisplay(mesoStartDate, AppSettings.appLocale) +
						qsTr("</b>  through  <b>") + JSF.formatDateToDisplay(mesoEndDate, AppSettings.appLocale) + "</b>"
				font.pixelSize: AppSettings.fontSizeLists
				Layout.alignment: Qt.AlignCenter
				Layout.maximumWidth: parent.width - 10
				Layout.leftMargin: 5
				Layout.bottomMargin: 2
			}
		}
	}

	/*WorkerScript {
		   id: worker
		   source: "populateMesoCalendar.js"
		   onMessage: function(progress) {
						  console.log(progress);
		   }
	}*/

	Popup {
		id: dlgProgressIndicator
		width: mainwindow.width - 100
		height: contentHeight + 20
		y: (mainwindow.height / 2) - 100
		x: (mainwindow.width / 2) - (width / 2)
		contentHeight: 2
		property var months_arr: []
		property var meso_id

		property bool bForward

		Timer {
			id: timer
			interval: 10
			running: false
			repeat: true
			property int i
			property int x

			onTriggered: {
				dlgProgressIndicator.progress ();
				if (i < dlgProgressIndicator.months_arr.length) {
					if (x < dlgProgressIndicator.months_arr[i].daySplit.length) {
						var calDate = new Date(dlgProgressIndicator.months_arr[i].yearNbr, dlgProgressIndicator.months_arr[i].monthNbr, dlgProgressIndicator.months_arr[i].daySplit[x].dayNbr);
						Database.newMesoCalendarEntry(dlgProgressIndicator.meso_id, calDate.getTime(), dlgProgressIndicator.months_arr[i].daySplit[x].trainingDayNumber,
														dlgProgressIndicator.months_arr[i].daySplit[x].daySplitLetter);
						x++;
					}
					else {
						x = 0;
						++i;
					}
				}
				else {
					running = false;
					dlgProgressIndicator.close ();
					appDBModified = true;
				}
			}
		}

		function init (message, from, to) {
			progressBar.from = from;
			progressBar.to = to;
			progressBar.value = from;
			bForward = true;
			lblMessage.text = message;
			dlgProgressIndicator.contentHeight += lblMessage.contentHeight;
			timer.i = 0;
			timer.x = 0;
			timer.start();
		}

		function progress () {
			if (bForward) {
				progressBar.value += 1;
				if (progressBar.value == progressBar.to)
					bForward = false;
			}
			else {
				progressBar.value -= 1;
				if (progressBar.value == progressBar.from)
					bForward = true;
			}
		}

		Column {
			id: mainLayout
			anchors.fill: parent
			spacing: 20

			Label {
				id: lblMessage
				Layout.alignment: Qt.AlignLeft
				Layout.margins: 10
				padding: 10
				width: dlgProgressIndicator.width - 10
				wrapMode: Text.Wrap
				font.bold: true
			}

			ProgressBar {
				id: progressBar
				width: dlgProgressIndicator.width - 20
				Layout.alignment: Qt.AlignLeft
				Layout.margins: 10

				background: Rectangle {
					implicitWidth: 200
					implicitHeight: 6
					color: paneBackgroundColor
					radius: 3
				}

				contentItem: Item {
					implicitWidth: dlgProgressIndicator.width - 20
					implicitHeight: 6

					Rectangle {
						width: progressBar.visualPosition * parent.width
						height: parent.height
						radius: 2
						color: "#e6e6e6"
					}
				}
			}

			Component.onCompleted: {
				dlgProgressIndicator.contentHeight += lblMessage.height + progressBar.height + 10;
			}
		} // Column
	} // Popup

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

		model: ListModel {
			id: mesoMonthsModel
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
					text: calendar.monthsNames[model.monthNbr] + " " + model.yearNbr;
					font.pixelSize: AppSettings.titleFontSizePixelSize
					font.bold: true
				}
			}

			DayOfWeekRow {
				id: weekTitles
				locale: monthGrid.locale
				anchors {
					top: monthYearTitle.bottom
				}
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
				month: model.monthNbr
				year: model.yearNbr
				spacing: 0
				anchors {
					top: weekTitles.bottom
				}
				width: parent.width
				height: calendar.cellSize * 8
				property var daysSplitModel: model.daySplit

				locale: Qt.locale(AppSettings.appLocale)

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
									if (monthGrid.daysSplitModel.get(model.day-1).isTrainingDay) {
										colorValue =  "steelblue";
										bIsTrainingDay = true;
									}
								}
							}
							bIsTrainingDay = false;
						}
						color = colorValue
					}

					Text {
						anchors.centerIn: parent
						text: monthGrid.month === model.month ? monthGrid.daysSplitModel.get(model.day-1).isTrainingDay ? model.day + "-" + monthGrid.daysSplitModel.get(model.day-1).daySplitLetter : model.day : model.day
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
								splitLetter = monthGrid.daysSplitModel.get(model.day-1).daySplitLetter;
								trainingDay = monthGrid.daysSplitModel.get(model.day-1).trainingDayNumber;
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

		Component.onCompleted: {
			mesoContentPage.StackView.activating.connect(pageActivation);
		}
	} // footer: ToolBar

	//Cannot call a PopUp when the parent object is not displayed. Since we need to update the database now
	//because all the alterations are based on memory models and the user may exit the application before they open this page,
	//therefore losing all the information
	function saveModelToDatabase(model) {
		var i = 0;
		while (i < model.length) {
			var x = 0;
			var len2 = model[i].daySplit.length;
			while (x < len2) {
				var calDate = new Date(model[i].yearNbr, model[i].monthNbr, model[i].daySplit[x].dayNbr);
				Database.newMesoCalendarEntry(mesoId, calDate.getTime(), model[i].daySplit[x].trainingDayNumber, model[i].daySplit[x].daySplitLetter);
				++x;
			}
			++i;
		}
	}

	function databaseChanged() {
		bReloadDatabase = true;
	}

	function readDatabase(bStopIfNoDB) {
		if (bVisualLoad) {
			let months, month;
			let calendar = Database.getMesoCalendar(mesoId);
			if ( calendar.length >= 1) {
				months = convertCalendarToMonthsModel(calendar);
			}
			else {
				if (bStopIfNoDB) return;
				months = getMesoMonths(mesoStartDate, mesoEndDate, mesoSplit);
				dlgProgressIndicator.months_arr = months;
				dlgProgressIndicator.meso_id = mesoId;
				dlgProgressIndicator.open();
				dlgProgressIndicator.init("Creating database. Please wait...", 0, 50);
				//worker.sendMessage({months_arr:months, meso_id:mesoId});
			}

			if (bReloadDatabase)
				mesoMonthsModel.clear();
			for (month of months) {
				mesoMonthsModel.append(month);
				//console.log(month.monthNbr + "  " + month.yearNbr + "   " + month.daySplit.length);
			}
			bReloadDatabase = false;
		}
	}

	function pageActivation() {
		if (bVisualLoad || bReloadDatabase) {
			readDatabase(false);
		}
	}

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
