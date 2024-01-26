import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import "jsfunctions.js" as JSF

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

	property color dividerColor: colorPalette[0]
	property color cardAndDialogBackground: colorPalette[1]
	property real primaryTextOpacity: colorPalette[2]
	property real secondaryTextOpacity: colorPalette[3]
	property real dividerOpacity: colorPalette[4]
	property real iconActiveOpacity: colorPalette[5]
	property real iconInactiveOpacity: colorPalette[6]
	property int isDarkTheme: colorPalette[8]
	property color flatButtonTextColor: colorPalette[9]
	property color popupTextColor: colorPalette[10]
	property real toolBarActiveOpacity: colorPalette[11]
	property real toolBarInactiveOpacity: colorPalette[12]
	property color toastColor: colorPalette[13]
	property real toastOpacity: colorPalette[14]
	property color paneBackgroundColor: primaryDarkColor //"#c1d0ce"

	property string lightIconFolder: darkPalette[7]
	property string darkIconFolder: lightPalette[7]
//	property var colorStyle: [Material.System, Material.Light, Material.Dark]

	property var materialBlue: ["#BBDEFB", "#25b5f3", "#1976D2", "#000000", "#FFFFFF", "#FFFFFF", "black", "white", "white"]
	property color primaryLightColor: materialBlue[0]
	property color primaryColor: materialBlue[1]
	property color primaryDarkColor: materialBlue[2]
	property color textOnPrimaryLight: materialBlue[3]
	property color textOnPrimary: materialBlue[4]
	property color textOnPrimaryDark: materialBlue[5]
	property string iconOnPrimaryLightFolder: materialBlue[6]
	property string iconOnPrimaryFolder: materialBlue[7]
	property string iconOnPrimaryDarkFolder: materialBlue[8]

	property var accentPalette: [materialBlue[1], materialBlue[4], materialBlue[7]]
	property color accentColor: accentPalette[0]
	property color textOnAccent: accentPalette[1]
	property string iconOnAccentFolder: accentPalette[2]

	property bool isLandscape: width > height
	property bool bNavButtonsEnabled: true
	property bool appDBModified: false
	property bool bLongTask: false

	property date todayFull
	property date today
	property int currentMesoIdx: -1

	property var trainingDayInfoPages: []
	property var mesoPlannerList: []
	property var dbExercisesListPage: null

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

				if ( initialPage.mainModel.count > 0 ) {
					currentMesoIdx = initialPage.mainModel.count - 1;
					if (initialPage.mainModel.get(currentMesoIdx).realMeso) {
						do {
							var mesoStartDate = initialPage.mainModel.get(currentMesoIdx).mesoStartDate;
							var mesoEndDate = initialPage.mainModel.get(currentMesoIdx).mesoEndDate;
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

	Timer {
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
	}

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
		contentItem.Keys.pressed.connect( function(event) {
			if (event.key === Qt.Key_Back) {
				event.accepted = true;
				androidBackKeyPressed();
			}
		});
	}

	function androidBackKeyPressed() {
		if (stackView.depth >= 2)
			stackView.pop();
		else
			close();
	}

	header: NavBar {
		id: navBar
		stackView: stackView

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		Component.onCompleted: {
			dateTimer.triggered();
			backupTimer.triggered();
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
			id: initialPage

			Component.onCompleted: Database.init_database();
		}

		StackView {
			id: stackView
			anchors.fill: parent
			initialItem: initialPage
		}
	}

	footer: TabBar {
		id: tabMain

		TabButton {
			text: qsTr("HOME")
			enabled: bNavButtonsEnabled && stackView.depth >= 2
			onClicked: {
				stackView.pop(stackView.get(0));
			}
		}

		TabButton {
			text: qsTr("  + Day")
			enabled: stackView.depth === 1 && currentMesoIdx >= 0
			Image {
				source: "qrc:/images/"+darkIconFolder+"exercises.png"
				height: 30
				width: 30
				anchors.verticalCenter: parent.verticalCenter
			}

			onClicked: { // Use most current meso
				for (var i = 0; i < trainingDayInfoPages.length; ++i) {
					if (trainingDayInfoPages[i].Object.mainDate === new Date(today)) {
						stackView.push(trainingDayInfoPages[i].Object, StackView.DontLoad);
						return;
					}
				}

				const mostRecentMeso = initialPage.mainModel.count - 1;
				const meso_name = initialPage.mainModel.get(mostRecentMeso).mesoName
				var tday = 0, splitletter = 'A', mesoid = 0;
				var bFirstTime = false;

				if (initialPage.mainModel.get(mostRecentMeso).realMeso) {
					let calendar_info = Database.getMesoCalendarDate(today);
					tday = calendar_info[0].mesoCalnDay;
					splitletter = calendar_info[0].mesoCalSplit;
					mesoid = calendar_info[0].mesoCalMesoId;
				}
				else {
					let meso_info = Database.getMesoInfo(initialPage.mainModel.get(mostRecentMeso).mesoId);
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

				var component = Qt.createComponent("TrainingDayInfo.qml");
				if (component.status === Component.Ready) {
					var trainingDayInfoPage = component.createObject(mainwindow, {
						mainDate: new Date(today), tDay: tday, splitLetter: splitletter,
						mesoName: meso_name, mesoId: mesoid, bFirstTime: bFirstTime
					});
					trainingDayInfoPages.push({ "Object":trainingDayInfoPage });
					stackView.push(trainingDayInfoPage, StackView.DontLoad);
				}
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
			var component = Qt.createComponent("ExercisesDatabase.qml");
			if (component.status === Component.Ready) {
				dbExercisesListPage = component.createObject(mainwindow, {});
			}
		}
		stackView.push(dbExercisesListPage, StackView.DontLoad);
	}
} //ApplicationWindow
