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
		imageSource: "back.png"
		hasDropShadow: false
		width: AppSettings.itemLargeHeight
		height: width
		enabled: ItemManager.AppPagesManager.currentIndex > 0

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.AppPagesManager.prevPage();
	}

	TPButton {
		id: btnForward
		imageSource: "next.png"
		hasDropShadow: false
		width: AppSettings.itemLargeHeight
		height: width
		enabled: ItemManager.AppPagesManager.currentIndex < ItemManager.AppPagesManager.count - 1

		anchors {
			left: btnBack.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.AppPagesManager.nextPage();
	}

	TPButton {
		id: btnHome
		imageSource: "home.png"
		hasDropShadow: false
		width: AppSettings.itemLargeHeight
		height: width
		enabled: btnBack.enabled

		anchors {
			left: btnForward.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.AppPagesManager.goHome();
	}

	TPButton {
		id: btnMainMenu
		imageSource: "mainmenu"
		hasDropShadow: false
		width: AppSettings.itemLargeHeight
		height: width

		anchors {
			verticalCenter: parent.verticalCenter
			right: parent.right
			rightMargin: 5
		}

		onClicked: ItemManager.AppPagesManager.openMainMenu();
	}

	TPButton {
		id: btnCalendar
		imageSource: "calendar"
		hasDropShadow: false
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
					_navBar.mainCalendar = component.createObject(ItemManager.AppMainWindow,
						{ parentPage: ItemManager.AppPagesManager.homePage(), showDate: new Date(), simpleCalendar: true,
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
		imageSource: "timer"
		hasDropShadow: false
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
					_navBar.mainTimer = component.createObject(ItemManager.AppMainWindow,
																	{ parentPage: ItemManager.AppPagesManager.homePage() });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			_navBar.mainTimer.showInWindow();
		}
	}

	TPButton {
		id: btnWeather
		imageSource: "weather"
		hasDropShadow: false
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
		imageSource: "exercisesdb"
		hasDropShadow: false
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
		imageSource: "statistics"
		hasDropShadow: false
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
