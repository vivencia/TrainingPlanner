import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
	id: dlgProgressIndicator
	width: mainwindow.width - 100
	height: contentHeight + 20
	y: (mainwindow.height / 2) - 100
	x: (mainwindow.width - width)/2

	property var calendarInfo: []
	property int calId
	property int splitIdx
	property int tDay
	property string mesoSplit
	property date mesoEndDate

	property string splitletter
	property bool bForward

	Timer {
		id: timer
		interval: 10
		running: false
		repeat: true
		property int i

		onTriggered: {
			dlgProgressIndicator.progress();
			if (i < dlgProgressIndicator.calendarInfo.length) {
				if (dlgProgressIndicator.calendarInfo[i].mesoCalDate <= dlgProgressIndicator.mesoEndDate.getTime()) {
					dlgProgressIndicator.calId = dlgProgressIndicator.calendarInfo[i].mesoCalId;
					dlgProgressIndicator.splitletter = mesoSplit.charAt(dlgProgressIndicator.splitIdx);
					Database.updateMesoCalendarDaySplit(dlgProgressIndicator.calId, dlgProgressIndicator.splitletter);
					if (dlgProgressIndicator.splitletter !== 'R')
						Database.updateMesoCalendarTrainingDay(dlgProgressIndicator.calId, dlgProgressIndicator.tDay++);
					else
						Database.updateMesoCalendarTrainingDay(dlgProgressIndicator.calId, 0);
					dlgProgressIndicator.splitIdx++;
					if (dlgProgressIndicator.splitIdx >= dlgProgressIndicator.mesoSplit.length)
						dlgProgressIndicator.splitIdx = 0;
					++i;
					return;
				}
				else {
					console.log("reached mesoenddate:", dlgProgressIndicator.calendarInfo[i].mesoCalDate, dlgProgressIndicator.mesoEndDate);
				}
			}
			running = false;
			dlgProgressIndicator.terminate();
		}
	}

	function init (message, from, to) {
		progressBar.from = from;
		progressBar.to = to;
		progressBar.value = from;
		bForward = true;
		lblMessage.text = message;
		dlgProgressIndicator.contentHeight += lblMessage.contentHeight;
		timer.i = 1;
		dlgProgressIndicator.calendarInfo = Database.getPartialMesoCalendar_From(calId);
		open();
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

	function terminate() {
		timer.stop ();
		close();
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
				}
			}
		}

		Component.onCompleted: {
			dlgProgressIndicator.contentHeight += lblMessage.height + progressBar.height + 10;
		}
	} // Column
} // Popup
