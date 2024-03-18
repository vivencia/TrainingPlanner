import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
	id: mainwindow
	objectName: "mainWindow"
	width: 300
	height: 640
	visible: true
	title: "Training Planner"

	signal backButtonPressed()
	signal mainMenuOpened()
	signal mainMenuClosed()
	signal appAboutToBeSuspended()
	signal appActive()

	property bool bLongTask: false
	property StackView appStackView: stackView

	readonly property color primaryLightColor: "#BBDEFB"
	readonly property color primaryColor: "#25b5f3"
	readonly property color primaryDarkColor: "#1976D2"
	readonly property string lightIconFolder: "white/"
	readonly property string darkIconFolder: "black/"

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

		StackView {
			id: stackView
			objectName: "appStackView"
			anchors.fill: parent
			initialItem: homePage
		}
	}

	footer: TabBar {
		id: tabMain

		TabButton {
			text: qsTr("HOME")
			enabled: appStackView.depth >= 2
			onClicked: {
				appStackView.pop(appStackView.get(0));
			}
		}

		TabButton {
			text: qsTr("  + Day")
			enabled: appStackView.depth === 1 //&& mesocyclesModel.count > 0
			Image {
				source: "qrc:/images/"+darkIconFolder+"exercises.png"
				height: 30
				width: 30
				anchors.verticalCenter: parent.verticalCenter
			}

			onClicked: {
				const today = new Date();
				var calendarPage;
				function pushTDayOntoMainStackView(object2, id) {
					if (id === 70) {
						appDB.getPage.disconnect(pushTDayOntoMainStackView);
						object2.tDay = mesoCalendarModel.getTrainingDay(today.getMonth() + 1, today.getDate() - 1);
						object2.splitLetter = mesoCalendarModel.getSplitLetter(today.getMonth() + 1, today.getDate() - 1);
						appStackView.push(object2, StackView.DontLoad);
					}
				}

				function mesoCalendarOK()
				{
					appDB.databaseReady.disconnect(mesoCalendarOK);
					appDB.getPage.connect(pushTDayOntoMainStackView);
					appDB.getTrainingDay(today);
				}

				if (mesoCalendarModel.count > 0)
					mesoCalendarOK();
				else {
					appDB.databaseReady.connect(mesoCalendarOK);
					appDB.getMesoCalendar(false);
				}
			} //onClicked
		} //TabButton
	} //footer

	function init() {
		if (mesocyclesModel.count !== 0)
			appDB.getMesoSplit();
		homePage.pageActivation();
	}
} //ApplicationWindow
