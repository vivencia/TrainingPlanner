import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "jsfunctions.js" as JSF

Page {
	id: trainingDayPage
	title: "trainingPage"

	required property date mainDate //dayDate
	required property int tDay //dayNumber
	required property string splitLetter //daySplitLetter
	required property string mesoName

	property int dayId: -1
	property int mesoId
	property string exercisesNames
	property string timeIn
	property string timeOut
	property string location
	property string trainingNotes

	property bool bModified: false
	property var exerciseSpriteList: []
	property var mesoSplit
	property var mesoSplitLetter
	property var mesoTDay;

	property bool bStopBounce: false
	property bool bNotScroll: true
	property bool bHasPreviousDay: false
	property bool bHasMesoPlan: false
	property date previousDivisionDayDate
	property int scrollBarPosition: 0
	property var navButtons: null

	property bool bShowSimpleExercisesList: false
	property var exerciseEntryThatRequestedSimpleList: null

	signal mesoCalendarChanged()

	onBModifiedChanged: {
		bNavButtonsEnabled = !bModified;
	}

	TimePicker {
		id: dlgTimeIn
		hrsDisplay: JSF.getHourOrMinutesFromStrTime (txtInTime.text)
		minutesDisplay: JSF.getMinutesOrSeconsFromStrTime (txtInTime.text)

		onTimeSet: (hour, minutes) => {
			timeIn = hour + ":" + minutes;
			bModified = true;
		}
	}

	TimePicker {
		id: dlgTimeOut
		hrsDisplay: JSF.getHourOrMinutesFromStrTime (txtOutTime.text)
		minutesDisplay: JSF.getMinutesOrSeconsFromStrTime (txtOutTime.text)

		onTimeSet: (hour, minutes) => {
			timeOut = hour + ":" + minutes;
			bModified = true;
			hideFloatingButton(-1); //Day is finished
			btnSaveDay.clicked();
		}
	}

	ScrollView {
		id: scrollTraining
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.height + colExercises.implicitHeight
		anchors.fill: parent

		ScrollBar.vertical.onPositionChanged: {
			if (bStopBounce) {
				if (ScrollBar.vertical.position < scrollBarPosition) {
					scrollToPos(scrollBarPosition);
				}
			}
			else {
				if (navButtons) {
					if (contentItem.contentY <= 50) {
						navButtons.showUpButton = false;
						navButtons.showDownButton = true;
					}
					else if (contentItem.contentY >= height + navBar.height) {
					//else if (scrollBarPosition - contentItem.contentY <= 70) { //This number is arbitrary, but was chosen after reviewing debugging information
						navButtons.showUpButton = true;
						navButtons.showDownButton = false;
					}
					else {
						navButtons.showUpButton = true;
						navButtons.showDownButton = true;
					}
				}
			}
		}

		ColumnLayout {
			id: colMain
			width: parent.width
			spacing: 0

			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
				topMargin: 5
			}

			Label {
				id: label1
				topPadding: 20
				bottomPadding: 20
				Layout.maximumWidth: parent.width - 10
				Layout.leftMargin: 5
				horizontalAlignment: Qt.AlignHCenter
				wrapMode: Text.WordWrap
				text: qsTr("Trainning day <b>#" + mesoTDay + "</b> of meso cycle <b>" + mesoName +
						"</b>: <b>" + JSF.formatDateToDisplay(mainDate, AppSettings.appLocale) + "</b> Division: <b>" + mesoSplitLetter + "</b>")
				font.pixelSize: AppSettings.fontSizeText
			}

			GridLayout {
				Layout.fillWidth: true
				columns: 2
				rows: 2
				Layout.leftMargin: 5
				Layout.rightMargin: 5

				Label {
					text: qsTr("Training Division:")
					Layout.row: 0
					Layout.column: 0
				}
				RegularExpressionValidator {
					id: regEx
					regularExpression: new RegExp(/[A-FR]+/);
				}
				TextField {
					id: txtSplitLetter
					text: splitLetter
					font.bold: true
					font.pixelSize: AppSettings.fontSizeText
					width: 40
					maximumLength: 1
					validator: regEx
					Layout.maximumWidth: 40
					Layout.row: 1
					Layout.column: 0

					onTextEdited: {
						if (acceptableInput) {
							bModified = true;
							splitLetter = text;
						}
					}
				} //txtSplitLetter

				Label {
					text: qsTr("Training Day #")
					Layout.row: 0
					Layout.column: 1
				}
				TextField {
					id: txtTDay
					text: tDay
					width: 50
					maximumLength: 3
					font.bold: true
					font.pixelSize: AppSettings.fontSizeText
					validator: IntValidator { bottom: 0; top: 180; }
					inputMethodHints: Qt.ImhDigitsOnly
					readOnly: splitLetter === 'R'
					Layout.maximumWidth: 40
					Layout.row: 1
					Layout.column: 1

					onTextEdited: {
						if ( text !== "") {
							if (parseInt(text) !== mesoTDay) {
								bModified = true;
								tDay = text;
							}
						}
					}
				} //txtTDay
			} //GridLayout

			Frame {
				id: frmMesoSplitAdjust
				Layout.fillWidth: true
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				visible: splitLetter !== mesoSplitLetter || tDay !== mesoTDay
				padding: 0
				spacing: 0

				ColumnLayout {
					id: layoutSplit
					anchors.fill: parent
					spacing: 0

					CheckBox {
						id: chkAdjustCalendar
						text: qsTr("Adjust meso calendar from the next day till the end?")
						checked: false
						Layout.maximumWidth: frmMesoSplitAdjust.width
						Layout.leftMargin: 5

						indicator: Rectangle {
							implicitWidth: 26
							implicitHeight: 26
							x: chkAdjustCalendar.leftPadding
							y: parent.height / 2 - height / 2
							radius: 5
							border.color: chkAdjustCalendar.down ? primaryDarkColor : primaryLightColor

							Rectangle {
								width: 14
								height: 14
								x: 6
								y: 6
								radius: 2
								color: chkAdjustCalendar.down ? primaryDarkColor : primaryLightColor
								visible: chkAdjustCalendar.checked
							}
						}

						contentItem: Text {
							text: chkAdjustCalendar.text
							wrapMode: Text.WordWrap
							opacity: enabled ? 1.0 : 0.3
							verticalAlignment: Text.AlignVCenter
							leftPadding: chkAdjustCalendar.indicator.width + chkAdjustCalendar.spacing
						}

						onClicked: {
							if (checked)
								optUpdateCalendarContinue.checked = true;
						}
					} //CheckBox

					RadioButton {
						id: optUpdateCalendarContinue
						text: qsTr("Continue cycle from this division letter")
						enabled: chkAdjustCalendar.checked
						checked: false
					}

					RadioButton {
						id: optUpdateCalendarStartOver
						text: qsTr("Start cycle over")
						checked: false
						enabled: chkAdjustCalendar.checked
					}
				} // ColumnLayout
			} //Frame

			Label {
				text: qsTr("Location:")
				Layout.leftMargin: 5
			}
			TextField {
				id: txtLocation
				placeholderText: "Academia Golden Era"
				text: location
				font.bold: true
				font.pixelSize: AppSettings.fontSizeText
				Layout.fillWidth: true
				Layout.rightMargin: 10
				Layout.leftMargin: 5

				onTextEdited: {
					if (text.length >= 4) {
						bModified = true;
						location = text;
					}
				}
			}

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5

				Label {
					id: lblInTime
					text: qsTr("In time:")
				}
				TextField {
					id: txtInTime
					text: timeIn !== "" ? timeIn : JSF.getTimeStringFromDateTime(todayFull)
					readOnly: true
					font.bold: true
					font.pixelSize: AppSettings.fontSizeText
					Layout.maximumWidth: txtInTime.text.contentWidth + 30
					Layout.leftMargin: 5
				}
				RoundButton {
					id: btnInTime
					icon.source: "qrc:/images/"+darkIconFolder+"time.png"

					onClicked: dlgTimeIn.open()
				}
			} //RowLayout

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5

				Label {
					id: lblOutTime
					text: qsTr("Out time:")
				}

				TextField {
					id: txtOutTime
					text: timeOut !== "" ? timeOut : JSF.getTimeStringFromDateTime(todayFull)
					font.bold: true
					font.pixelSize: AppSettings.fontSizeText
					readOnly: true
					Layout.maximumWidth: txtOutTime.text.contentWidth + 30
					Layout.leftMargin: 5
				}

				RoundButton {
					id: btnOutTime
					icon.source: "qrc:/images/"+darkIconFolder+"time.png"

					onClicked: dlgTimeOut.open()
				}
			} // Row

			Label {
				id: lblDayInfoTrainingNotes
				text: qsTr("This training session considerations:")
				Layout.leftMargin: 5
			}
			Flickable {
				Layout.fillWidth: true
				Layout.rightMargin: 5
				Layout.leftMargin: 5
				height: Math.min(contentHeight, 60)
				contentHeight: txtDayInfoTrainingNotes.implicitHeight

				TextArea.flickable: TextArea {
					id: txtDayInfoTrainingNotes
					text: trainingNotes
					font.bold: true
					font.pixelSize: AppSettings.fontSizeText

					onEditingFinished: {
						if (text.length >= 4) {
							bModified = true;
							trainingNotes = text;
						}
					}
				}
				ScrollBar.vertical: ScrollBar {}
				ScrollBar.horizontal: ScrollBar {}
			} //Flickable

			Label {
				id: lblExercisesStart
				text: qsTr("--- EXERCISES ---")
				font.bold: true
				Layout.alignment: Qt.AlignCenter
				Layout.bottomMargin: 2
			}

			GroupBox {
				id: grpIntent
				label: Label {
					text: qsTr("What do you want to do today?")
					anchors.horizontalCenter: parent.horizontalCenter
					anchors.bottomMargin: 10
				}
				visible: bHasMesoPlan || bHasPreviousDay
				width: parent.width - 20
				Layout.fillWidth: true
				Layout.bottomMargin: 30
				property int option
				spacing: 0
				padding: 0

				ColumnLayout {
					anchors.fill: parent
					spacing: 0

					RadioButton {
						id: optMesoPlan
						padding: 0
						contentItem: Label {
							text: qsTr("Use the standard exercises plan for the division ") + mesoSplitLetter + qsTr(" of the Mesocycle")
							wrapMode: Text.WordWrap
							font.pixelSize: AppSettings.fontSizeText
							anchors.left: parent.left
							anchors.leftMargin: 25
							anchors.right: parent.right
							anchors.rightMargin: 5
						}

						visible: bHasMesoPlan
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						indicator: Rectangle {
							implicitWidth: 20
							implicitHeight: 20
							x: 3
							y: parent.height / 2 - height / 2
							radius: 10
							border.color: optMesoPlan.down ? primaryDarkColor : primaryLightColor

							Rectangle {
								width: 14
								height: 14
								x: 3
								y: 3
								radius: 7
								color: optMesoPlan.down ? primaryDarkColor : primaryLightColor
								visible: optMesoPlan.checked
							}
						}

						onClicked: {
							grpIntent.option = 0;
						}
					}
					RadioButton {
						id: optPreviousDay
						padding: 0
						contentItem: Label {
							text: qsTr("Base this session off the one from ") + JSF.formatDateToDisplay(previousDivisionDayDate, AppSettings.appLocale)
							wrapMode: Text.WordWrap
							font.pixelSize: AppSettings.fontSizeText
							anchors.left: parent.left
							anchors.leftMargin: 25
							anchors.right: parent.right
							anchors.rightMargin: 5
						}
						visible: bHasPreviousDay
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						indicator: Rectangle {
							implicitWidth: 20
							implicitHeight: 20
							x: 3
							y: parent.height / 2 - height / 2
							radius: 10
							border.color: optPreviousDay.down ? primaryDarkColor : primaryLightColor

							Rectangle {
								width: 14
								height: 14
								x: 3
								y: 3
								radius: 7
								color: optPreviousDay.down ? primaryDarkColor : primaryLightColor
								visible: optPreviousDay.checked
							}
						}

						onClicked: {
							grpIntent.option = 1;
						}
					}
					RadioButton {
						id: optEmptySession
						padding: 0
						contentItem: Label {
							text: qsTr("Start an empty session")
							font.pixelSize: AppSettings.fontSizeText
							anchors.left: parent.left
							anchors.leftMargin: 25
							anchors.right: parent.right
							anchors.rightMargin: 5
						}
						visible: bHasMesoPlan || bHasPreviousDay
						width: parent.width
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignLeft

						indicator: Rectangle {
							implicitWidth: 20
							implicitHeight: 20
							x: 3
							y: parent.height / 2 - height / 2
							radius: 10
							border.color: optEmptySession.down ? primaryDarkColor : primaryLightColor

							Rectangle {
								width: 14
								height: 14
								x: 3
								y: 3
								radius: 7
								color: optEmptySession.down ? primaryDarkColor : primaryLightColor
								visible: optEmptySession.checked
							}
						}

						onClicked: {
							grpIntent.option = 2;
						}
					}

					ButtonFlat {
						id: btnChooseIntent
						text: qsTr("Begin")
						Layout.alignment: Qt.AlignCenter

						onClicked: {
							switch (grpIntent.option) {
								case 0: //use meso plan
									bHasPreviousDay = false;
									loadTrainingDayInfoFromMesoPlan();
								break;
								case 1: //use previous day
									bHasMesoPlan = false;
									loadTrainingDayInfo(previousDivisionDayDate);
									dayId = -1;
								break;
								case 2: //empty session
									bHasPreviousDay = false;
									bHasMesoPlan = false;
								break;
							}
							grpIntent.visible = false;
						}
					}
				}
			}
		}// colMain

		ColumnLayout {
			id: colExercises
			width: parent.width

			anchors {
				left: parent.left
				leftMargin: 5
				rightMargin: 5
				right:parent.right
				top: colMain.bottom
			}
		}

		Item {
			id: phantomItem
			width: parent.width
			height: 10

			anchors {
				left: parent.left
				right:parent.right
				bottom: parent.bottom
			}
		}

		Component.onCompleted: {
			loadOrCreateDayInfo();
		}

		function scrollToPos(y_pos) {
			contentItem.contentY = y_pos;
			if (navButtons === null) {
				var component = Qt.createComponent("PageScrollButtons.qml");
				navButtons = component.createObject(this, {});
				navButtons.scrollTo.connect(setScrollBarPosition);
				navButtons.backButtonWasPressed.connect(maybeShowNavButtons);
			}
			navButtons.visible = true;
		}

		function setScrollBarPosition(pos) {
			bNotScroll = true;
			if (pos === 0)
				ScrollBar.vertical.setPosition(0);
			else
				ScrollBar.vertical.setPosition(pos - ScrollBar.vertical.size/2);
		}
	} // ScrollView scrollTraining

	Timer {
		id: bounceTimer
		interval: 200
		running: false
		repeat: false

		onTriggered: {
			bStopBounce = false;
			bNotScroll = false;
		}
	}

	function loadOrCreateDayInfo() {
		let mesoinfo = Database.getMesoInfo(mesoId);
		if ( mesoinfo.length > 0 ) {
			mesoSplit = mesoinfo[0].mesoSplit;
		}
		mesoSplitLetter = splitLetter;
		mesoTDay = tDay;
		if (!loadTrainingDayInfo(mainDate)) {
			checkIfMesoPlanExists();
			checkIfPreviousDayExists();
		}
	}

	function createDatabaseEntryForDay() {
		let result = Database.newTrainingDay(mainDate.getTime(), mesoId, exercisesNames,
			tDay, splitLetter, txtInTime.text, txtOutTime.text, txtLocation.placeholderText, txtDayInfoTrainingNotes.text);
		dayId = result.insertId;
		bModified = false;
	}

	function getMesoPlan () {
		let plan_info = [];
		switch (mesoSplitLetter) {
			case 'A': plan_info = Database.getCompleteDivisionAForMeso(mesoId); break;
			case 'B': plan_info = Database.getCompleteDivisionBForMeso(mesoId); break;
			case 'C': plan_info = Database.getCompleteDivisionCForMeso(mesoId); break;
			case 'D': plan_info = Database.getCompleteDivisionDForMeso(mesoId); break;
			case 'E': plan_info = Database.getCompleteDivisionEForMeso(mesoId); break;
			case 'F': plan_info = Database.getCompleteDivisionFForMeso(mesoId); break;
		}
		return plan_info;
	}

	function checkIfMesoPlanExists() {
		let plan_info = getMesoPlan();
		if (plan_info.length > 0) {
			if (plan_info[0].splitExercises)
				bHasMesoPlan = plan_info[0].splitExercises.length > 1;
		}
	}

	function checkIfPreviousDayExists() {
		let day_info = Database.getPreviousTrainingDayForDivision(mesoSplitLetter, tDay, mesoId);
		for (var i = 0; i < day_info.length; ++i ) { //from the most recent to the oldest
			if (day_info[i].exercisesNames) {
				bHasPreviousDay = day_info[i].exercisesNames.length > 1;
				if (bHasPreviousDay) {
					previousDivisionDayDate = new Date(day_info[i].dayDate);
					break;
				}
			}
			if (!bHasPreviousDay)
				Database.deleteTraingDay(day_info[i].dayId); //remove empty day from DB
		}
	}

	function loadTrainingDayInfoFromMesoPlan() {
		let plan_info = getMesoPlan();
		exercisesNames = plan_info[0].splitExercises;
		createExercisesFromList();

		const types = plan_info[0].splitSetTypes.split('|');
		const nsets = plan_info[0].splitNSets.split('|');
		const nreps = plan_info[0].splitNReps.split('|');
		const nweights = plan_info[0].splitNWeight.split('|');
		for(var i = 0; i < exerciseSpriteList.length; ++i)
			exerciseSpriteList[i].Object.createSetsFromPlan(nsets[i], types[i], nreps[i], nweights[i]);
		bModified = true;
	}

	function loadTrainingDayInfo(tDate) {
		console.log("Trying to load info for the day: ", tDate.toDateString(), tDate.getTime());
		let dayInfoList = Database.getTrainingDay(tDate.getTime());
		if (dayInfoList.length > 0) {
			if (dayInfoList[0].exercisesNames === null) {//Day is saved but it is empty. Treat it as if it weren't saved then
				Database.deleteTraingDay(dayInfoList[0].dayId);
				return false;
			}
			dayId = dayInfoList[0].dayId;
			var tempMesoId = dayInfoList[0].mesoId;
			if (tempMesoId !== mesoId) {
				//The information recorded for the day has a mesoId that refers to a mesocycle that does not
				//match the passed mesoId. Since all the ways to get to this page must go through methods that check
				//the valilidy of a mesoId, the stored mesoId is wrong and must be replaced
				Database.updateTrainingDay_MesoId(dayId, mesoId);
			}
			exercisesNames = dayInfoList[0].exercisesNames;
			timeIn = dayInfoList[0].dayTimeIn;
			timeOut = dayInfoList[0].dayTimeOut;
			location = dayInfoList[0].dayLocation;
			trainingNotes = dayInfoList[0].dayNotes;
			createExercisesFromList();
			bModified = true;
			return true;
		}
		return false;
	}

	function updateDayIdFromExercisesAndSets() {
		for( var i = 0; i < exerciseSpriteList.length; ++i )
			exerciseSpriteList[i].Object.updateDayId(dayId);
		bModified = true;
	}

	function gotExercise(strName1, strName2, sets, reps, weight, bAdd) {
		if (bAdd) {
			strName1 += ' - ' + strName2;
			addExercise(strName1);
			strName2 = "";
		}

		var component;
		var exerciseSprite;
		component = Qt.createComponent("ExerciseEntry.qml");

		if (component.status === Component.Ready) {
			var idx = exerciseSpriteList.length;
			exerciseSprite = component.createObject(colExercises, {thisObjectIdx:idx, exerciseName:strName1,
				exerciseName1:strName1, exerciseName2:strName2, tDayId:dayId, stackViewObj:trainingDayPage.StackView.view});
			exerciseSpriteList.push({"Object" : exerciseSprite});
			exerciseSprite.exerciseRemoved.connect(removeExercise);
			exerciseSprite.exerciseEdited.connect(editExercise);
			exerciseSprite.setAdded.connect(addExerciseSet);
			exerciseSprite.requestHideFloatingButtons.connect(hideFloatingButton);

			bStopBounce = true;
			scrollBarPosition = phantomItem.y - trainingDayPage.height + 2*exerciseSprite.height;
			if (scrollBarPosition >= 0)
				scrollTraining.scrollToPos(scrollBarPosition);
			bounceTimer.start();
			if (navButtons !== null)
				navButtons.visible = true;
		}
		else
			console.log("not ready");
	}

	function addExerciseSet(bnewset, sety, setheight, exerciseObjIdx) {
		if (bnewset) {
			bModified = true;
			bStopBounce = true;
			scrollBarPosition = phantomItem.y - trainingDayPage.height + exerciseSpriteList[exerciseObjIdx].Object.height;
			if (exerciseObjIdx === exerciseSpriteList.length-1) {
				scrollBarPosition = phantomItem.y - trainingDayPage.height + exerciseSpriteList[exerciseObjIdx].Object.height + setheight;
			}
			else {
				scrollBarPosition = phantomItem.y - trainingDayPage.height + exerciseSpriteList[exerciseObjIdx].Object.height - sety - setheight;
			}
			//console.log(exerciseSpriteList[exerciseObjIdx].Object.height);
			//console.log(exerciseObjIdx);
			//console.log(scrollBarPosition);
			scrollTraining.scrollToPos(scrollBarPosition);
			bounceTimer.start();
		}
	}

	function editExercise(exerciseIdx, newExerciseName) {
		bModified = true;
		if (exerciseIdx !== -1) {
			const names = exercisesNames.split('|');
			exercisesNames = "";
			for (var i = 0; i < names.length; ++i) {
				if (i !== exerciseIdx)
					exercisesNames += names[i] + '|';
				else
					exercisesNames += newExerciseName + '|';
			}
			exercisesNames = exercisesNames.slice(0, -1);
		}
	}

	function removeExercise(objidx) {
		let newObjectList = new Array;

		for( var i = 0, x = 0; i < exerciseSpriteList.length; ++i ) {
			if (i === objidx) {
				removeExerciseName(objidx);
				exerciseSpriteList[objidx].Object.destroy();
			}
			else {
				newObjectList[x] = exerciseSpriteList[i];
				newObjectList[x].Object.thisObjectIdx = x;
				newObjectList[x].Object.updateSetsExerciseIndex();
				x++;
			}
		}
		delete exerciseSpriteList;
		exerciseSpriteList = newObjectList;
		bModified = true;
	}

	function addExercise(exercisename) {
		if (exercisesNames.length > 0)
			exercisesNames += '|' + exercisename;
		else
			exercisesNames = exercisename;
		bModified = true;
	}

	function removeExerciseName(exerciseIdx) {
		const names = exercisesNames.split('|');
		exercisesNames = "";
		for (var i = 0; i < names.length; ++i) {
			if (i !== exerciseIdx)
				exercisesNames += names[i] + '|';
		}
		exercisesNames = exercisesNames.slice(0, -1);
		bModified = true;
	}

	function createExercisesFromList() {
		const names = exercisesNames.split('|');
		var sep, name, name2 = "";
		for (var i = 0; i < names.length; ++i) {
			sep = names[i].indexOf('&');
			if (sep !== -1) { //Composite exercise
				name = names[i].substring(0, sep);
				name2 = names[i].substring(sep + 1, names[i].length);
			}
			else
				name = names[i];
			gotExercise(name, name2, "0", "0", "0", false);
			exerciseSpriteList[exerciseSpriteList.length-1].Object.bFoldPaneOnLoad = true;
		}
	}

	function updateMesoCalendar() {
		if (mesoSplitLetter !== splitLetter || mesoTDay !== tDay) {
			var calendar_date_info;
			var cal_id;
			calendar_date_info = Database.getMesoCalendarDate(mainDate);
			if (calendar_date_info.length > 0) {
				cal_id = calendar_date_info[0].mesoCalId;
				Database.updateMesoCalendarDaySplit(cal_id, splitLetter);

				//If in the old calendar, day was R or the user did not explicitly change tDay, go back
				//in date until we find a suitable training day. This will be starting training day if the user
				//wants to modify the rest of the calendar, or simply the continuation from the previous training day if not
				if (mesoSplitLetter === 'R' || tDay === mesoTDay) {
					//Find a suitable training day to continue
					var prevdate = JSF.getPreviousDate(mainDate);
					//Find the first training day before mainDate that is not a rest day;
					while ( (calendar_date_info = Database.getMesoCalendarDate(prevdate)) !== null) {
						//console.log(prevdate.toDateString());
						//console.log(calendar_date_info[0].mesoCalSplit);
						if (calendar_date_info[0].mesoCalSplit !== 'R') {
							tDay = calendar_date_info[0].mesoCalnDay + 1;
							break;
						}
						prevdate = JSF.getPreviousDate(prevdate);
					}
				}
				//Update this day
				Database.updateMesoCalendarTrainingDay(cal_id, tDay);
				if (chkAdjustCalendar.checked) {
					var split_idx;
					var tday;
					var splitletter;
					if (optUpdateCalendarContinue.checked) {
						split_idx = mesoSplit.indexOf(splitLetter);
						split_idx++;
						if (split_idx >= mesoSplit.length)
							split_idx = 0;

						tday = tDay + 1; //tDay of the next day in calendar
					}
					else { //Start all over
						Database.updateMesoCalendarTrainingDay(cal_id, splitLetter === 'R' ? 0 : 1); //Update this day
						split_idx = 0;
						tday = splitLetter === 'R' ? 1 : 2; //tDay of the next day in calendar
					}

					let result = Database.getMesoInfo(mesoId);
					if (result.length === 1) {
						const mesoenddate = new Date(result[0].mesoEndDate);
						startProgressDialog(cal_id, split_idx, tday, mesoenddate);
					}
				} // chkAdjustCalendar.checked = true

				//Hide the frame for adjustments
				mesoSplitLetter = splitLetter;
				mesoTDay = tDay;
				mesoCalendarChanged(); //Signals MesoContent.qml to reread from the database
			} //calendar_date_info.length > 0
		} //mesoSplitLetter !== splitLetter
	} // updateMesoCalendar()

	function startProgressDialog(calid, splitidx, day, mesoenddate) {
		var component;
		var progressSprite;
		component = Qt.createComponent("TrainingDayProgressDialog.qml");

		if (component.status === Component.Ready) {
			progressSprite = component.createObject(trainingDayPage, {calId:calid, splitIdx:splitidx, tDay:day,
							mesoSplit:mesoSplit, mesoEndDate:mesoenddate } );
			progressSprite.init(qsTr("Updating calendar database. Please wait..."), 0, 30);
		}
	}

	function hideFloatingButton(except_idx) {
		for (var x = 0; x < exerciseSpriteList.length; ++x) {
			if (x !== except_idx) {
				if (exerciseSpriteList[x].Object.bFloatButtonVisible)
					exerciseSpriteList[x].Object.bFloatButtonVisible = false;
			}
		}
	}

	function maybeShowNavButtons() {
		navButtons.showButtons();
	}

	footer: ToolBar {
		id: dayInfoToolBar
		width: parent.width
		visible: !bShowSimpleExercisesList

		ToolButton {
			id: btnSaveDay
			enabled: bModified
			anchors.left: parent.left
			anchors.leftMargin: 5
			anchors.verticalCenter: parent.verticalCenter
			display: AbstractButton.TextUnderIcon
			text: qsTr("Log Day")
			font.capitalization: Font.MixedCase

			icon.source: "qrc:/images/"+lightIconFolder+"save-day.png"
			icon.height: 20
			icon.width: 20

			onClicked: {
				if (location === "")
					location = txtLocation.placeholderText;
				if (trainingNotes === "")
					trainingNotes = " ";
				if (splitLetter === 'R')
					tDay = 0;
				updateMesoCalendar();
				if (dayId === -1) {
					createDatabaseEntryForDay();
					if (bHasMesoPlan || bHasPreviousDay)
						updateDayIdFromExercisesAndSets();
				}
				Database.updateTrainingDay(dayId, exercisesNames, tDay, splitLetter, timeIn, timeOut, location, trainingNotes);

				for (var i = 0; i < exerciseSpriteList.length; ++i)
						exerciseSpriteList[i].Object.logSets();
				bModified = false;
				appDBModified = true;
			}
		} //btnSaveDay

		ToolButton {
			id: btnRevertDay
			enabled: bModified
			anchors.left: btnSaveDay.right
			anchors.verticalCenter: parent.verticalCenter
			display: AbstractButton.TextUnderIcon
			text: qsTr("Cancel alterations")
			font.capitalization: Font.MixedCase

			icon.source: "qrc:/images/"+lightIconFolder+"revert-day.png"
			icon.height: 20
			icon.width: 20

			onClicked: {
				if (navButtons !== null)
					navButtons.visible = false;
				const len = exerciseSpriteList.length - 1;
				for (var i = len; i >= 0; --i) {
					exerciseSpriteList[i].Object.destroy();
					exerciseSpriteList.pop();
				}
				loadOrCreateDayInfo();
				bModified = false;
			}
		} //btnRevertDay

		ToolButton {
			id: btnAddExercise
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Add exercise")
			font.capitalization: Font.MixedCase
			display: AbstractButton.TextUnderIcon
			icon.source: "qrc:/images/"+lightIconFolder+"exercises-add.png"
			icon.width: 20
			icon.height: 20
			enabled: !grpIntent.visible

			onClicked: {
				hideFloatingButton(-1);
				if (navButtons !== null)
					navButtons.hideButtons();

				var exercise = trainingDayPage.StackView.view.push("ExercisesDatabase.qml", { bChooseButtonEnabled: true });
				exercise.exerciseChosen.connect(gotExercise);
			}
		} // bntAddExercise
	} //footer: ToolBar

	ExercisesListView {
		id: exercisesList
		visible: bShowSimpleExercisesList
		height: mainwindow.height * 0.4
		width: parent.width
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath) => {
			if (exerciseEntryThatRequestedSimpleList)
				exerciseEntryThatRequestedSimpleList.changeExercise(exerciseName, subName);
		}
	}

	function requestSimpleExerciseList(object) {
		bShowSimpleExercisesList = true;
		exerciseEntryThatRequestedSimpleList = object;
		scrollTraining.setScrollBarPosition(1);
	}

	function closeSimpleExerciseList() {
		bShowSimpleExercisesList = false;
		exerciseEntryThatRequestedSimpleList = null;
	}
} // Page
