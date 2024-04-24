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

	header: NavBar {
		id: navBar

		background: Rectangle {
			color: AppSettings.primaryDarkColor
			opacity: 0.7
		}
	}

	MainMenu {
		id: mainMenu
		objectName: "appMainMenu"
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
			enabled: stackView.depth >= 2

			Image {
				source: "qrc:/images/"+darkIconFolder+"home.png"
				height: 30
				width: 30
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.left
				anchors.leftMargin: 10
			}

			onClicked: stackView.pop(stackView.get(0));
		}

		TabButton {
			text: qsTr("   + Workout")
			enabled: stackView.depth === 1

			Image {
				source: "qrc:/images/"+darkIconFolder+"exercises.png"
				height: 40
				width: 40
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.left
				anchors.leftMargin: 10
			}

			onClicked: {
				const today = new Date();
				var calendarPage;
				function pushTDayOntoMainStackView(object2, id) {
					if (id === 70) {
						appDB.getPage.disconnect(pushTDayOntoMainStackView);
						mainMenu.addShortCut( qsTr("Workout: ") + runCmd.formatDate(today) , object2);
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

	function androidBackKeyPressed() {
		if (stackView.depth >= 2)
			stackView.pop();
		else
			close();
	}

	function init() {
		if (mesocyclesModel.count !== 0)
			appDB.getMesoSplit();
		homePage.pageActivation();
	}
} //ApplicationWindow
