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

	property var trainingDayInfoPages: []
	property var dbExercisesListPage: null
	readonly property var appStackView: stackView

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

	/*Timer {
		id: backupTimer
		interval: 600000 //Every ten minutes
		running: true
		repeat: true

		onTriggered: {
			if (appDBModified) {
				backUpClass.doBackUp();
				console.log("database modified!!!!");
			}
			else
				console.log("database not modified!!!");
		}
	}*/

/*case Qt.ApplicationSuspended: console.info("###########  Application Suspended ##############"); break;
					case Qt.ApplicationHidden: console.info("###########  Application Hidden ##############"); break;
					case Qt.ApplicationInactive: console.info("###########  Application Inactive ##############"); break;
					case Qt.ApplicationActive: console.info("###########  Application Active ##############"); break;*/
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
	}

	Component.onDestruction: {
		if (dbExercisesListPage !== null)
			dbExercisesListPage.destroy();
		var i = 0;
		for (; i < trainingDayInfoPages.length; ++i)
			trainingDayInfoPages[i].Object.destroy();
	}

	function androidBackKeyPressed() {
		if (appStackView.depth >= 2)
			appStackView.pop();
		else
			close();
	}

	function loadExercises() {
		appDB.pass_object(exercisesListModel);
		appDB.getAllExercises();
	}

	header: NavBar {
		id: navBar

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		Component.onCompleted: {
			dateTimer.triggered();
			//backupTimer.triggered();
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

		/*DBMesoSplitModel {
			id: mesosplitmodel
		}
		MesoSplitPlanner {
			id: mesoSplit
			mesoId: 1
			mesoIdx: 0
			splitLetter: "A"
			splitModel: mesosplitmodel
		}*/

		StackView {
			id: stackView
			anchors.fill: parent
			initialItem: homePage//mesoSplit
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

			onClicked: { // Use most current meso
				for (var i = 0; i < trainingDayInfoPages.length; ++i) {
					if (trainingDayInfoPages[i].date === today.getTime()) {
						appStackView.push(trainingDayInfoPages[i].Object, StackView.DontLoad);
						return;
					}
				}

				const mostRecentMeso = mesocyclesModel.count - 1;
				const meso_name = mesocyclesModel.get(mostRecentMeso, 1);
				var tday = 0, splitletter = 'A', mesoid = 0;
				var bFirstTime = false;

				if (mesocyclesModel.get(mostRecentMeso, 8) === "1") {
					let calendar_info = Database.getMesoCalendarDate(today);
					tday = calendar_info[0].mesoCalnDay;
					splitletter = calendar_info[0].mesoCalSplit;
					mesoid = calendar_info[0].mesoCalMesoId;
				}
				else {
					let meso_info = Database.getMesoInfo(mesocyclesModel.getInt(mostRecentMeso, 0));
					const mesosplit = meso_info[0].mesoSplit;
					let day_info = Database.getMostRecentTrainingDay();
					if (day_info.exercisesNames) {
						tday = day_info[0].dayNumber + 1;
						splitletter = getNextLetterInSplit(mesosplit, day_info[0].mesoCalSplit);
					}
					else {
						tday = 1;
						splitletter = mesosplit.length > 0 ? mesosplit.charAt(0) : 'A';
						bFirstTime = true;
					}
					mesoid = meso_info[0].mesoId;
				}
				/*Database.getAllTrainingDays(mesoid);
				let res = Database.getPreviousTrainingDayForDivision(splitletter, tday, mesoid);
				for( var x = 0; x < res.length; x++) {
					console.info("%%%%%%%%%% ", res[x].dayId);
					console.info("%%%%%%%%%% ", new Date(res[x].dayDate).toDateString());
					console.info("%%%%%%%%%% ", res[x].exercisesNames);
					console.info("%%%%%%%%%% ", res[x].dayNumber);
					console.info("%%%%%%%%%% ", res[x].daySplitLetter);
				}
				return;*/

				var component = Qt.createComponent("TrainingDayInfo.qml", Qt.Asynchronous);

				function finishCreation() {
					var trainingDayInfoPage = component.createObject(mainwindow, {
						mainDate: today, tDay: tday, splitLetter: splitletter,
						mesoName: meso_name, mesoId: mesoid, bFirstTime: bFirstTime
					});

					//Maximum of 3 pages loaded on memory. The latest page replace the earliest
					if (trainingDayInfoPages.length === 3) {
						trainingDayInfoPages[0].Object.destroy();
						trainingDayInfoPages[0] = trainingDayInfoPages[1];
						trainingDayInfoPages[1] = trainingDayInfoPages[2];
						trainingDayInfoPages.pop();
					}

					trainingDayInfoPages.push({ "date": today.getTime(), "Object":trainingDayInfoPage });
					appStackView.push(trainingDayInfoPage, StackView.DontLoad);
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			} //onClicked
		} //TabButton
	} //footer

	function getNextLetterInSplit(mesosplit, currentletter) {
		if (mesosplit.length > 0) {
			const idx = mesosplit.indexOf(currentletter);
			if (idx >= 0) {
				++idx;
				if (idx >= mesosplit.length)
					idx = 0;
				return mesosplit.charAt(idx);
			}
		}
		return 'A';
	}

	function openDbExercisesListPage() {
		var option;
		var component;
		function finishCreation() {
			console.log("tooltip should disappear now")
			dbExercisesListPage = component.createObject(mainwindow, { "width":mainwindow.width, "height":mainwindow.height });
			appStackView.push(dbExercisesListPage, StackView.DontLoad);
		}

		function readyToProceed() {
			appDB.qmlReady.disconnect(readyToProceed);
			if (option === 0)
			{
				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			else
				appStackView.push(dbExercisesListPage, StackView.DontLoad);
		}

		appDB.qmlReady.connect(readyToProceed);
		if (!dbExercisesListPage) {
			component = Qt.createComponent("ExercisesDatabase.qml", Qt.Asynchronous);
			option = 0;
			if (exercisesListModel.count === 0) {
				console.log("tooltip should appear now")
				loadExercises();
			}
		}
		else {
			option = 1;
			if (exercisesListModel.count === 0) {
				console.log("tooltip should appear now")
				loadExercises();
			}
			else
				readyToProceed();
		}
	}
} //ApplicationWindow
