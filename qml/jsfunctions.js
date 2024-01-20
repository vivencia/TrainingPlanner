function getMonthFromDateString(strDate) {
	var month_part = strDate.substr(4, 3);
	if (month_part === qsTr("Jan"))
		return 0;
	if (month_part === qsTr("Feb"))
		return 1;
	if (month_part === qsTr("Mar"))
		return 2;
	if (month_part === qsTr("Apr"))
		return 3;
	if (month_part === qsTr("May"))
		return 4;
	if (month_part === qsTr("Jun"))
		return 5;
	if (month_part === qsTr("Jul"))
		return 6;
	if (month_part === qsTr("Aug"))
		return 7;
	if (month_part === qsTr("Sep"))
		return 8;
	if (month_part === qsTr("Oct"))
		return 9;
	if (month_part === qsTr("Nov"))
		return 10;
	return 11;
}

function getYearFromDateString(strDate) {
	var year_part = strDate.length === 15 ? strDate.substr(11, 4) : strDate.substr(10, 4);
	return parseInt(year_part);
}

function getDayFromDateString(strDate) {
	var day_part = strDate.substr(8, 2);
	return parseInt(day_part);
}

function getDateFromDateString(strDate) {
	var newdate = new Date (getYearFromDateString(strDate), getMonthFromDateString(strDate), getDayFromDateString(strDate));
	return newdate;
}

function weekNumber(date1) {
	var startDate = new Date(date1.getFullYear(), 0, 1);
	var days = Math.floor((date1 - startDate) /
		(24 * 60 * 60 * 1000));
	var n = Math.ceil(days / 7);
	//console.log("Week number for date " + date1 + ": " + n)
	return n;
}

function calculateNumberOfWeeks (week1, week2) {
	var n;
	//Every 6 years we have 53 week year
	if ( week2 < week1 ) {
		var totalWeeksInYear = new Date().year !== 2026 ? 52 : 53
		n = (totalWeeksInYear - week1) + week2;
	}
	else
		n = week2 - week1;
	//console.log("Number of weeks between (" + week1 + " and " + week2 + "): " + n+1)
	return n+1; //+1 include current week
}

function getMonthTotalDays(month,year) {
	switch (month) {
		case 0:
		case 2:
		case 4:
		case 6:
		case 7:
		case 9:
		case 11:
			return 31;
		case 1:
		case 5:
		case 8:
		case 10:
			return 30;
		case 1:
			if (year % 4 === 0)
				return 29;
			else
				return 28;
	}
}

function createFutureDate(currentDate, year, month, day) {
	var f_day = currentDate.getDate() + day;
	var f_month = currentDate.getMonth () + month;
	var f_year = currentDate.getFullYear() + year;

	var totaldays = getMonthTotalDays(currentDate.getMonth ());
	if (f_day > totaldays) {
		f_month++;
		f_day = f_day - totaldays;
	}
	if ( f_month >= 12 ) {
		f_year++;
		f_month = f_month - 12;
	}
	return new Date (f_year, f_month, f_day);
}

function getPreviousDate(date) {
	var year = date.getFullYear();
	var month = date.getMonth();
	var day = date.getDate() - 1;
	if (day === 0) {
		month--;
		if (month === -1) {
			month = 11;
			year--;
		}
		day = getMonthTotalDays(month,year);
	}
	return new Date(year,month,day);
}

function getNextDate(date) {
	var year = date.getFullYear();
	var month = date.getMonth();
	var day = date.getDate() + 1;
	if (day > getMonthTotalDays(month,year)) {
		month++;
		if (month === 12) {
			month = 0;
			year++;
		}
		day = 1;
	}
	return new Date(year,month,day);
}

function formatDateToDisplay(date, locale) {
	if (locale === "pt_BR") {
		switch (Qt.platform.os) {
			case "android":
				return date.toLocaleString(locale).slice(0, 8);
			case "linux":
				return date.toLocaleString(locale).slice(0, 10);
		}
	}
	else {
		return date.toDateString();
	}
}

//time format: hh:mm, HH:mm, mm:ss
function getHourOrMinutesFromStrTime(time) {
	if (time.length > 3) {
		var hour_part = time.substr(0, time.indexOf(":"));
		return hour_part;
	}
	else {
		return new Date().getHours().toString();
	}
}

//time format: hh:mm, HH:mm, mm:ss
function getMinutesOrSeconsFromStrTime(time) {
	if (time.length > 3) {
		var min_part = time.substr(time.indexOf(":") + 1, time.length );
		return min_part;
	}
	else {
		return new Date().getMinutes().toString();
	}
}

function getTimeStringFromDateTime (datetime) {
	return createStrTimeFromInts(datetime.getHours(), datetime.getMinutes());
}

//Accepts hours and minutes or minutes and seconds
function createStrTimeFromInts(hour, mins) {
	var strHour = hour.toString();
	if (strHour.length === 1 )
		strHour = "0" + strHour;
	var strMin = mins.toString();
	if (strMin.length === 1 )
		strMin = "0" + strMin;
	return strHour + ":" + strMin;
}

//currentTime format: mm:ss, additionalTime format: mm:ss
//Not much checking be cause the inputs are all done programatically and not by the user
function increaseStringTimeBy(currentTime, additionalTime) {
	var secs = parseInt(getMinutesOrSeconsFromStrTime(currentTime));
	var mins = parseInt(getHourOrMinutesFromStrTime(currentTime));
	const add_secs = parseInt(getMinutesOrSeconsFromStrTime(additionalTime));
	const add_mins = parseInt(getHourOrMinutesFromStrTime(additionalTime));

	secs += add_secs;
	if (secs > 59) {
		secs -= 60;
		mins++;
	}
	mins += add_mins;
	return createStrTimeFromInts(mins, secs);
}

function intTimeToStrTime(inttime) {
	if (inttime >=0 && inttime <=9)
		return "0" + inttime.toString();
	return inttime.toString();
}

function calculateTimeBetweenTimes(time1, time2) {
	const hour1 = getHourOrMinutesFromStrTime(time1);
	const min1 = getMinutesOrSeconsFromStrTime(time1);
	const hour2 = getHourOrMinutesFromStrTime(time2);
	const min2 = getMinutesOrSeconsFromStrTime(time2);

	var hour = hour2 - hour1;
	var min = min2 - min1
	if (min < 0) {
		hour--;
		min += 60;
	}
	return createStrTimeFromInts(hour, min);
}

function checkWhetherCanCreatePlan()
	{
	var ok = true;
	if (mesoSplit.indexOf('A') !== -1) {
		ok &= (txtSplitA.length > 1);
		txtSplitA.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('B') !== -1) {
		ok &= (txtSplitB.length > 1);
		txtSplitB.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('C') !== -1) {
		ok &= (txtSplitC.length > 1);
		txtSplitC.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('D') !== -1) {
		ok &= (txtSplitD.length > 1);
		txtSplitD.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('E') !== -1) {
			ok &= (txtSplitE.length > 1);
		txtSplitE.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('F') !== -1) {
		ok &= (txtSplitF.length > 1);
		txtSplitF.cursorPosition = 0;
	}
	btnCreateExercisePlan.enabled = ok;
}

function moveFocusToNextField(from) {
	switch (from) {
		case '0':
			if (txtSplitA.visible)
				txtSplitA.forceActiveFocus();
			else if (txtSplitB.visible)
				txtSplitB.forceActiveFocus();
			else if (txtSplitC.visible)
				txtSplitC.forceActiveFocus();
			else if (txtSplitD.visible)
				txtSplitD.forceActiveFocus();
			else if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				txtMesoDrugs.forceActiveFocus();
		break;
		case 'A':
			if (txtSplitB.visible)
				txtSplitB.forceActiveFocus();
			else if (txtSplitC.visible)
				txtSplitC.forceActiveFocus();
			else if (txtSplitD.visible)
				txtSplitD.forceActiveFocus();
			else if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				txtMesoDrugs.forceActiveFocus();
		break;
		case 'B':
			if (txtSplitC.visible)
				txtSplitC.forceActiveFocus();
			else if (txtSplitD.visible)
				txtSplitD.forceActiveFocus();
			else if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				txtMesoDrugs.forceActiveFocus();
		break;
		case 'C':
			if (txtSplitD.visible)
				txtSplitD.forceActiveFocus();
			else if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				txtMesoDrugs.forceActiveFocus();
		break;
		case 'D':
			if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				txtMesoDrugs.forceActiveFocus();
		break;
		case 'E':
			if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				txtMesoDrugs.forceActiveFocus();
		break;
		case 'F':
			txtMesoDrugs.forceActiveFocus();
		break;
	}
}
