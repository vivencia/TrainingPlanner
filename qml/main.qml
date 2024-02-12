import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import "jsfunctions.js" as JSF
import com.vivenciasoftware.qmlcomponents

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

	property var darkPalette: ["#FFFFFF", "#424242", "1.0", "0.70", "0.12", "1.0", "0.3", "white/", "1", "#FFFFFF", "#FFFFFF", "1.0", "0.7", "Darkgrey", "0.9"]
	property var lightPalette: ["#000000", "#FFFFFF", "0.87", "0.54", "0.12", "0.54", "0.26", "black/", "0", "#424242", "#424242", "1.0", "0.7", "#323232", "0.75"]
	property var colorPalette: AppSettings.themeStyleColorIndex === 2 ? darkPalette : lightPalette

	property real iconActiveOpacity: colorPalette[5]
	property real iconInactiveOpacity: colorPalette[6]
	property color paneBackgroundColor: primaryDarkColor //"#c1d0ce"

	property string lightIconFolder: darkPalette[7]
	property string darkIconFolder: lightPalette[7]
//	property var colorStyle: [Material.System, Material.Light, Material.Dark]

	property var materialBlue: ["#BBDEFB", "#25b5f3", "#1976D2", "#000000", "#FFFFFF", "#FFFFFF", "black", "white", "white"]
	property color primaryLightColor: materialBlue[0]
	property color primaryColor: materialBlue[1]
	property color primaryDarkColor: materialBlue[2]
	property color listEntryColor1: "#dce3f0"
	property color listEntryColor2: "#c3cad5"

	property var accentPalette: [materialBlue[1], materialBlue[4], materialBlue[7]]
	property color accentColor: accentPalette[0]
	property color textOnAccent: accentPalette[1]
	property string iconOnAccentFolder: accentPalette[2]

	property bool bNavButtonsEnabled: true
	property bool appDBModified: false
	property bool bLongTask: false

	property date todayFull
	property date today
	property int currentMesoIdx: -1

	property var trainingDayInfoPages: []
	property var mesoPlannerList: []
	property var dbExercisesListPage: null
	property var homePage: null
	readonly property var appStackView: stackView

	property var setTypes: [ { text:qsTr("Regular"), value:0 }, { text:qsTr("Pyramid"), value:1 }, { text:qsTr("Drop Set"), value:2 },
							{ text:qsTr("Cluster Set"), value:3 }, { text:qsTr("Giant Set"), value:4 }, { text:qsTr("Myo Reps"), value:5 } ]

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

				if ( DBMesocyclesModel.count > 0 ) {
					currentMesoIdx = DBMesocyclesModel.count - 1;
					if (DBMesocyclesModel.get(currentMesoIdx).realMeso) {
						do {
							var mesoStartDate = DBMesocyclesModel.get(currentMesoIdx, 2);
							var mesoEndDate = DBMesocyclesModel.get(currentMesoIdx, 3)
							if (today >= mesoStartDate && today <= mesoEndDate)
								break;
						} while (--currentMesoIdx >= 0);
					}
				}
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
		for (i = 0; i < mesoPlannerList.length; ++i)
			mesoPlannerList[i].Object.destroy();
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
			//backupTimer.triggered();
		}
	}

	MainMenu {
		id: mainMenu
	}

	DBExercisesModel {
		id:	exercisesListModel
	}

	DBMesocyclesModel {
		id: mesocyclesListModel

		Component.onCompleted: {
			var component = Qt.createComponent("HomePage.qml", Qt.Asynchronous);

			function finishCreation() {
				homePage = component.createObject(mainwindow, { "width":mainwindow.width, "height":mainwindow.height });
				stackView.initialItem = Qt.binding(function() { return homePage; })
			}

			function readyToCreate() {
				appDB.qmlReady.disconnect(readyToCreate);
				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			appDB.qmlReady.connect(readyToCreate);
			appDB.pass_object(mesocyclesListModel);
			appDB.getAllMesocycles();
		}
	}

	Flickable {
		width: parent.width
		height: parent.height
		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator {}

		//HomePage {
		//	id: initialPage
			//Component.onCompleted: Database.init_database();
		//}

		StackView {
			id: stackView
			anchors.fill: parent
			//initialItem: initialPage
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
			enabled: appStackView.depth === 1 && currentMesoIdx >= 0
			Image {
				source: "qrc:/images/"+darkIconFolder+"exercises.png"
				height: 30
				width: 30
				anchors.verticalCenter: parent.verticalCenter
			}

			onClicked: { // Use most current meso
				//Database.deleteTraingDay(13);
				//Database.deleteSets();
				//Database.getAllSetsInfo();
				//return;
				for (var i = 0; i < trainingDayInfoPages.length; ++i) {
					if (trainingDayInfoPages[i].date === today.getTime()) {
						appStackView.push(trainingDayInfoPages[i].Object, StackView.DontLoad);
						return;
					}
				}

				const mostRecentMeso = DBMesocyclesModel.count - 1;
				const meso_name = DBMesocyclesModel.get(mostRecentMeso).mesoName
				var tday = 0, splitletter = 'A', mesoid = 0;
				var bFirstTime = false;

				if (DBMesocyclesModel.get(mostRecentMeso).realMeso) {
					let calendar_info = Database.getMesoCalendarDate(today);
					tday = calendar_info[0].mesoCalnDay;
					splitletter = calendar_info[0].mesoCalSplit;
					mesoid = calendar_info[0].mesoCalMesoId;
				}
				else {
					let meso_info = Database.getMesoInfo(DBMesocyclesModel.get(mostRecentMeso).mesoId);
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
		if (!dbExercisesListPage) {
			var component = Qt.createComponent("ExercisesDatabase.qml", Qt.Asynchronous);

			function finishCreation() {
				console.log("tooltip should disappear now")
				dbExercisesListPage = component.createObject(mainwindow, { "width":mainwindow.width, "height":mainwindow.height });
				appStackView.push(dbExercisesListPage, StackView.DontLoad);
			}

			function readyToCreate() {
				console.log("waiting for component to be ready for creation")
				appDB.qmlReady.disconnect(readyToCreate);
				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			console.log("tooltip should appear now")
			appDB.qmlReady.connect(readyToCreate);
			appDB.pass_object(exercisesListModel);
			appDB.getAllExercises();
		}
		else
			appStackView.push(dbExercisesListPage, StackView.DontLoad);
	}
} //ApplicationWindow
