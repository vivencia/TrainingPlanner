import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import com.vivenciasoftware.qmlcomponents

ApplicationWindow {
	id: mainwindow
	objectName: "mainWindow"
	width: windowWidth
	height: windowHeight
	visible: true
	title: "Training Planner"

	signal backButtonPressed()
	signal mainMenuOpened()
	signal mainMenuClosed()
	signal appAboutToBeSuspended()
	signal appActive()

	property bool bNavButtonsEnabled: true
	property bool bLongTask: false

	property date todayFull
	property date today

	property StackView appStackView: stackView

	Timer {
		id: dateTimer
		interval: 30000
		running: true
		repeat: true

		onTriggered: {
			const newdate = new Date();
			if (newdate != todayFull) {
				todayFull = newdate; //This includes the time of object creation
				today = new Date(todayFull.getFullYear(), todayFull.getMonth(), todayFull.getDate()); //Normalized date of today with time always set to 0
			}
		}
	}

	BusyIndicator {
		id: busyIndicator
		running: bLongTask
		parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
		x: (mainwindow.width - width) / 2;
		y: (mainwindow.height - height) / 2;
	}

	Connections {
		target: Qt.application;
		function onStateChanged(inState) {
			if (Qt.platform.os === "android") {
				switch (inState) {
					case Qt.ApplicationSuspended: break;
					case Qt.ApplicationHidden: break;
					case Qt.ApplicationInactive: appAboutToBeSuspended(); break;
					case Qt.ApplicationActive: appActive(); break;
				}
				console.info("####  ", Qt.application.state);
			}
		}
	}

	onAfterSynchronizing: {
		if (bLongTask) {
			busyIndicator.visible = true;
		}
	}

	Component.onCompleted: {
		if (Qt.platform.os === "android") {
			contentItem.Keys.pressed.connect( function(event) {
				if (event.key === Qt.Key_Back) {
					event.accepted = true;
					androidBackKeyPressed();
				}
			});
		}
		appDB.setAppStackView(appStackView);
	}

	function androidBackKeyPressed() {
		if (appStackView.depth >= 2)
			appStackView.pop();
		else
			close();
	}

	header: NavBar {
		id: navBar

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		Component.onCompleted: {
			dateTimer.triggered();
		}
	}

	MainMenu {
		id: mainMenu
	}

	Flickable {
		width: parent.width
		height: parent.height
		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator {}

		HomePage {
			id: homePage
		}

		/*DBTrainingDayModel {
			id: tdaymodel
			currentRow: 0
		}
		TrainingDayInfo {
			id: tDayInfo
			mesoId: 1
			mesoIdx: 0
			modelIdx: 0
			mainDate: new Date()
			tDayModel: tdaymodel
		}*/

		StackView {
			id: stackView
			objectName: "appStackView"
			anchors.fill: parent
			initialItem: homePage //tDayInfo
		}
	}

	footer: TabBar {
		id: tabMain

		TabButton {
			text: qsTr("HOME")
			enabled: bNavButtonsEnabled && appStackView.depth >= 2
			onClicked: {
				appStackView.pop(appStackView.get(0));
			}
		}

		TabButton {
			text: qsTr("  + Day")
			enabled: appStackView.depth === 1 && mesocyclesModel.count > 0
			Image {
				source: "qrc:/images/"+darkIconFolder+"exercises.png"
				height: 30
				width: 30
				anchors.verticalCenter: parent.verticalCenter
			}

			onClicked: {
				var id;
				var calendarPage;
				function pushOntoStackView(object2, _id2) {
					if (id !== _id2) {
						appDB.getItem.disconnect(pushOntoStackView);
						object2.tDay = calendarPage.mesoCalendarModel.getTrainingDay(today.getMonth() + 1, today.getDate() - 1);
						object2.splitLetter = calendarPage.mesoCalendarModel.getSplit(today.getMonth() + 1, today.getDate() - 1);
						appStackView.push(object, StackView.DontLoad);
					}
				}

				function readyToProceed(object, _id)
				{
					appDB.getItem.disconnect(readyToProceed);
					id = _id;
					calendarPage = object;
					appDB.getItem.connect(pushOntoStackView);
					appDB.getTrainingDay(today);
				}

				appDB.getItem.connect(readyToProceed);
				appDB.getMesoCalendar();
			} //onClicked
		} //TabButton
	} //footer
} //ApplicationWindow
