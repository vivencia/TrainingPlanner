import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

Rectangle {
	id: _control
	radius: width * 0.5
	border.color: "green"
	border.width: workoutFinished ? 2 : 0
	opacity: _workout_day ? 1 : _meso_day ?  0.7 : 0.4
	color: _day_is_visible ? AppSettings.primaryLightColor : "transparent"
	height: cellSize
	width: cellSize

//public:
	required property int entryDay
	required property int entryMonth
	required property int entryYear
	required property int parentMonth
	required property int cellSize
	required property date today

	property DBCalendarModel tpCalendarModel
	property CalendarModel qtCalendarModel

	signal dateSelected(int day, int month, int year, bool is_workout);

//private:
	readonly property date month_day: new Date(entryYear, entryMonth, entryDay);
	readonly property bool _today_date: month_day.getUTCFullYear() === today.getUTCFullYear() && month_day.getUTCMonth() ===
																		today.getUTCMonth() && month_day.getUTCDate() === today.getUTCDate()
	readonly property bool _day_is_visible: entryMonth === parentMonth
	readonly property bool _meso_day: tpCalendarModel ? tpCalendarModel.isPartOfMeso(month_day) : true
	readonly property bool _workout_day: tpCalendarModel ? tpCalendarModel.isWorkoutDay(month_day) : false
	property bool workoutFinished: tpCalendarModel ? tpCalendarModel.completed_by_date(month_day) : false

	function highlightDay(highlighted: bool): void {
		if (highlighted)
			animExpand.start();
		else
			animShrink.start();
	}

	Component.onCompleted: {
		if (_today_date)
			dateSelected(_control.entryDay, _control.entryMonth, _control.entryYear, _workout_day);
	}

	Connections {
		enabled: _control.tpCalendarModel !== null
		target: _control.tpCalendarModel
		function onCompletedChanged(cal_date: date) : void {
			if (cal_date === _control.month_day)
				_control.workoutFinished = _control.tpCalendarModel.completed_by_date(cal_date);
		}
	}

	TPLabel {
		id: txtDay
		anchors.centerIn: parent
		text: _control.tpCalendarModel ? _control.tpCalendarModel.dayEntryLabel(_control.month_day) : parseInt(_control.entryDay)
		font: _control.tpCalendarModel ? AppGlobals.regularFont : AppGlobals.smallFont
		visible: _control._day_is_visible
		color: !_control._today_date ? (_control._meso_day ? AppSettings.fontColor : AppSettings.disabledFontColor) : "red"

		Connections {
			enabled: _control.tpCalendarModel !== null
			target: _control.tpCalendarModel
			function onSplitLetterChanged(cal_date: date) : void {
				if (cal_date === _control.month_day)
					txtDay.text = _control.tpCalendarModel.dayEntryLabel(cal_date);
			}
		}
	}

	SequentialAnimation { // Expand the button
		id: animExpand
		alwaysRunToEnd: true

		PropertyAnimation {
			target: _control
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
			target: _control
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}
	}

	MouseArea {
		anchors.fill: parent
		hoverEnabled: true

		onClicked:
			_control.dateSelected(_control.entryDay, _control.entryMonth, _control.entryYear, _control._workout_day);
	}
}
