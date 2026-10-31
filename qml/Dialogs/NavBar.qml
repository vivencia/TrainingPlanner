import QtQuick

import TpQml
import TpQml.Widgets

TPToolBar {
	id: _navBar
	height: AppSettings.windowHeight - AppSettings.pageHeight

	property PagesListModel pagesModel
	property CalendarDialog mainCalendar: null
	property TimerDialog mainTimer: null

	TPButton2 {
		id: btnBack
		image: "back.png"
		imageWidth: AppSettings.itemLargeHeight
		imageHeight: imageWidth
		enabled: ItemManager.appPagesManager.currentIndex > 0

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.appPagesManager.prevPage();
	}

	TPButton2 {
		id: btnForward
		image: "next.png"
		imageWidth: AppSettings.itemLargeHeight
		imageHeight: imageWidth
		enabled: ItemManager.appPagesManager.currentIndex < ItemManager.appPagesManager.count - 1

		anchors {
			left: btnBack.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.appPagesManager.nextPage();
	}

	TPButton2 {
		id: btnHome
		image: "home.png"
		imageWidth: AppSettings.itemLargeHeight
		imageHeight: imageWidth
		enabled: btnBack.enabled

		anchors {
			left: btnForward.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: ItemManager.appPagesManager.goHome();
	}

	TPButton2 {
		id: btnMainMenu
		image: "mainmenu"
		imageWidth: AppSettings.itemLargeHeight
		imageHeight: imageWidth

		anchors {
			verticalCenter: parent.verticalCenter
			right: parent.right
			rightMargin: 5
		}

		onClicked: ItemManager.appPagesManager.openMainMenu();
	}

	TPButton2 {
		id: btnCalendar
		image: "calendar"
		imageWidth: AppSettings.itemLargeHeight
		imageHeight: imageWidth

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

	TPButton2 {
		id: btnTimer
		image: "timer"
		imageWidth: AppSettings.itemLargeHeight
		imageHeight: imageWidth

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnCalendar.left
		}

		onButtonClicked: click();

		function click() {
			if (_navBar.mainTimer === null) {
				let component = Qt.createComponent("TpQml.Dialogs", TimerDialog, Qt.Asynchronous);

				function finishCreation() {
					_navBar.mainTimer = component.createObject(ItemManager.appHomePage(), {});
					click();
				}

				if (component.status === Component.Ready) {
					finishCreation();
				} else {
					component.statusChanged.connect(finishCreation);
					return;
				}
			}
			_navBar.mainTimer.tpQmlOpen(ItemManager.appHomePage());
		}
	}

	TPButton2 {
		id: btnWeather
		image: "weather"
		imageWidth: AppSettings.itemLargeHeight
		imageHeight: imageWidth
		enabled: AppOsInterface.internetOK

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnTimer.left
		}

		onClicked: ItemManager.getWeatherPage();
	}

	TPButton2 {
		id: btnExercisesList
		image: "exercisesdb"
		imageWidth: AppSettings.itemLargeHeight
		imageHeight: imageWidth

		anchors {
			verticalCenter: parent.verticalCenter
			right: btnWeather.left
		}

		onClicked: ItemManager.getExercisesPage();
	}
}
