import QtQuick
import QtQuick.Controls

import "../TPWidgets"
import "../Pages"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPToolBar {
	id: root
	height: appSettings.windowHeight - appSettings.pageHeight

	property CalendarDialog mainCalendar: null
	property TimerDialog mainTimer: null

	TPButton {
		id: btnBack
		imageSource: "back.png"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width
		enabled: stackView.depth >= 2

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: parent.verticalCenter
		}

		onClicked: pagesListModel.prevPage();
	}

	TPButton {
		id: btnForward
		imageSource: "next.png"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width
		enabled: pagesListModel.currentIndex < pagesListModel.count - 1

		anchors {
			left: btnBack.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: pagesListModel.nextPage();
	}

	TPButton {
		id: btnHome
		imageSource: "home.png"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width
		enabled: btnBack.enabled

		anchors {
			left: btnForward.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: goHome();
	}

	TPButton {
		id: btnMainMenu
		imageSource: "mainmenu"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: parent.right
			rightMargin: 5
		}

		onClicked: mainwindow.openMainMenu();
	}

	TPButton {
		id: btnCalendar
		imageSource: "calendar"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnMainMenu.left
			rightMargin: 10
		}

		onClicked: {
			if (mainCalendar === null) {
				let component = Qt.createComponent("qrc:/qml/Dialogs/CalendarDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					mainCalendar = component.createObject(mainwindow, { parentPage: homePage, showDate:new Date(),
						simpleCalendar:true, initDate: new Date(2000, 0, 1), finalDate: new Date(2030, 11, 31) });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			mainCalendar.open();
		}
	}

	TPButton {
		id: btnTimer
		imageSource: "timer"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnCalendar.left
			rightMargin: 10
		}

		onClicked: {
			if (mainTimer === null) {
				let component = Qt.createComponent("qrc:/qml/Dialogs/TimerDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					mainTimer = component.createObject(mainwindow, { parentPage: homePage, simpleTimer:true });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			mainTimer.open();
		}
	}

	TPButton {
		id: btnWeather
		imageSource: "weather"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnTimer.left
			rightMargin: 10
		}

		onClicked: itemManager.getWeatherPage();
	}

	TPButton {
		id: btnExercisesList
		imageSource: "exercisesdb"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnWeather.left
			rightMargin: 10
		}

		onClicked: itemManager.getExercisesPage();
	}

	TPButton {
		imageSource: "statistics"
		hasDropShadow: false
		width: appSettings.itemLargeHeight
		height: width

		enabled: {
			switch (appMesoModel().count) {
				case 0: return false;
				case 1: return !appMesoModel().isNewMeso();
				default: return true;
			}
		}

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnExercisesList.left
			rightMargin: 10
		}

		onClicked: itemManager.getStatisticsPage();
	}

	Component.onDestruction: {
		if (mainCalendar !== null)
			mainCalendar.destroy();
		if (mainTimer !== null)
			mainTimer.destroy();
	}
}
