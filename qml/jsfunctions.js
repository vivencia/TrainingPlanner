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
				return date.toLocaleString(locale).slice(0, 12);
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

function getTrainingDayNumber(selDate, startDate) {
    //Number of days from the beginning of meso to the end of first month
    var nday = getMonthTotalDays(startDate.getMonth(), startDate.getFullYear()) - startDate.getDate() + 1;

    //Add to it the month's total number of days
    var month;
    if (selDate.getMonth() > startDate.getMonth()) {
        for (month = startDate.getMonth() + 1; month < selDate.getMonth(); month++) {
            nday += getMonthTotalDays (month, startDate.getYear());
        }
        //Add the days from the selected month
        nday += selDate.getDate() + 1;
    }
    else if (selDate.getMonth() < startDate.getMonth()){
        var endmonth = 11 + selDate.getMonth() + 1;
        var year = startDate.getFullYear();
        //Go to the end of the year
        for (month = startDate.getMonth() + 1; month <= endmonth; month++) {
            if (month <= 11)
                nday += getMonthTotalDays (month, year);
            //When end of year is reached, reset everything and start from month 0
            else {
                month = 0;
                year++;
                endmonth = selDate.getMonth();
            }
        }
        //Add the days from the selected month
        nday += selDate.getDate() + 1;
    }
    else { //Selected month is the start month
        nday = (selDate.getDate() + 1) - startDate.getDate() + 1;
    }
    return nday;
}

function urlToPath(url) {
	var path
	if (url.startsWith("file:///")) {
		var k = url.charAt(9) === ':' ? 8 : 7
		path = url.substring(k)
	} else {
		path = url
	}
	return decodeURIComponent(path);
}

function pathToLocalUrl(path) {
	var str_path;
	if (path.length <= 7)
		str_path = "qrc:/images/no_image.jpg";
	else {
		if (!path.startsWith("qrc"))
			str_path = "file://" + path;
		else
			str_path = path;
	}
	var url = new URL(str_path);
	return url;
}

function toCompositeExerciseId(exercise1, exercise2) {
	if (exercise2 > 0)
		return exercise1 + exercise2 * 10000;
	else
		return exercise1;
}

function getExerciseIdFromCompositeExerciseId(idx, exerciseid) {
	var ret;
	switch (idx) {
		case 0: //first id
			if (exerciseid > 9999)
				ret = exerciseid % 10000;
			else
				ret = exerciseid;
		break;
		case 1: //second id
			if (exerciseid > 9999)
				ret = Math.floor(exerciseid / 10000);
			else
				ret = -1;
		break;
	}
	return ret;
}

