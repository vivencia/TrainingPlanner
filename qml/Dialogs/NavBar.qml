import QtQuick

import TpQml
import TpQml.Widgets

TPToolBar {
	id: _navBar
	height: AppSettings.windowHeight - AppSettings.pageHeight

	property PagesListModel pagesModel
	property CalendarDialog mainCalendar: null
	property TimerDialog mainTimer: null

	TPButton {
		id: btnBack
		image: "back.png"
		width: AppSettings.itemLargeHeight
		height: width
		enabled: ItemManager.appPagesManager.currentIndex > 0

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.appPagesManager.prevPage();
	}

	TPButton {
		id: btnForward
		image: "next.png"
		width: AppSettings.itemLargeHeight
		height: width
		enabled: ItemManager.appPagesManager.currentIndex < ItemManager.appPagesManager.count - 1

		anchors {
			left: btnBack.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.appPagesManager.nextPage();
	}

	TPButton {
		id: btnHome
		image: "home.png"
		width: AppSettings.itemLargeHeight
		height: width
		enabled: btnBack.enabled

		anchors {
			left: btnForward.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.appPagesManager.goHome();
	}

	TPButton {
		id: btnMainMenu
		image: "mainmenu"
		width: AppSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: parent.right
			rightMargin: 5
		}

		onClicked: ItemManager.appPagesManager.openMainMenu();
	}

	TPButton {
		id: btnCalendar
		image: "calendar"
		width: AppSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnMainMenu.left
			rightMargin: 10
		}

		onClicked: {
			if (_navBar.mainCalendar === null) {
				let component = Qt.createComponent("TpQml.Dialogs", CalendarDialog, Qt.Asynchronous);

				function finishCreation() {
					_navBar.mainCalendar = component.createObject(ItemManager.appMainWindow,
						{ parentPage: ItemManager.appPagesManager.homePage(), showDate: new Date(), simpleCalendar: true,
													initDate: new Date(2000, 0, 1), finalDate: new Date(2030, 11, 31) });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			_navBar.mainCalendar.tpOpen();
		}
	}

	TPButton {
		id: btnTimer
		image: "timer"
		width: AppSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnCalendar.left
			rightMargin: 10
		}

		onClicked: {
			if (_navBar.mainTimer === null) {
				let component = Qt.createComponent("TpQml.Dialogs", TimerDialog, Qt.Asynchronous);

				function finishCreation() {
					_navBar.mainTimer = component.createObject(ItemManager.appMainWindow, {});
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			_navBar.mainTimer.tpQmlOpen(ItemManager.appPagesManager.homePage());
		}
	}

	TPButton {
		id: btnWeather
		image: "weather"
		width: AppSettings.itemLargeHeight
		height: width
		enabled: AppOsInterface.internetOK

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnTimer.left
			rightMargin: 10
		}

		onClicked: ItemManager.getWeatherPage();
	}

	TPButton {
		id: btnExercisesList
		image: "exercisesdb"
		width: AppSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnWeather.left
			rightMargin: 10
		}

		onClicked: ItemManager.getExercisesPage();
	}

	TPButton {
		image: "statistics"
		width: AppSettings.itemLargeHeight
		height: width

		enabled: false

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnExercisesList.left
			rightMargin: 10
		}

		onClicked: ItemManager.getStatisticsPage();
	}
}
