import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "ExercisesAndSets"
import "Dialogs"
import "Pages"
import "TPWidgets"
import "User"

ApplicationWindow {
	id: mainwindow
	visible: true
	title: "TraininPlanner Tests"
	objectName: "mainwindow"
	width: appSettings.windowWidth
	height: appSettings.windowHeight
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint | Qt.WA_KeepScreenOn :
				Qt.Window | Qt.CustomizeWindowHint & ~Qt.WindowMaximizeButtonHint

	signal pageActivated_main(Item page);
	signal pageDeActivated_main(Item page);
	signal passwordDialogClosed(resultCode: int, password: string);
	signal saveFileChosen(filepath: string);
	signal saveFileRejected(filepath: string);
	signal openFileChosen(filepath: string, content_type: int);
	signal openFileRejected(filepath: string);

	property PagesListModel appPagesModel
	//Component.onCompleted: timePicker.show1();

	Connections {
		target: itemManager
		function onCppDataForQMLReady() : void {
			//lstWorkoutExercises.exercisesModel = itemManager.workoutModel();
			//lstWorkoutExercises.currentIndex = itemManager.workoutModel().workingExercise;
		}
	}

	TPPage {
		id: homePage
		objectName: "homePage"
		anchors.fill: parent
		signal mesosViewChanged(bool own_mesos);

		/*property MesocyclesModel mesoModel: null

		WorkoutOrSplitExercisesList {
			id: lstWorkoutExercises
			pageManager: homePage
			anchors.fill: parent
		}*/

		/*MediaControls {
			id: mediaControls
			height: appSettings.itemDefaultHeight
			width: homePage.width * 0.8
			availableControls: [ MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Prev, MediaControls.CT_Next,
				MediaControls.CT_Equalizer, MediaControls.CT_Rewind, MediaControls.CT_FastForward, MediaControls.CT_Mute ]
			onControlPressed: (type) => {
				switch (type) {
					case MediaControls.CT_Play: console.log("Play - pressed"); break;
					case MediaControls.CT_Pause: console.log("Pause - pressed"); break;
					case MediaControls.CT_Stop: console.log("Stop - pressed"); break;
					case MediaControls.CT_Prev: console.log("Prev - pressed"); break;
					case MediaControls.CT_Next: console.log("Next - pressed"); break;
					case MediaControls.CT_Equalizer: console.log("Equalizer - pressed"); break;
					case MediaControls.CT_Rewind: console.log("Rewind - pressed"); break;
					case MediaControls.CT_FastForward: console.log("Fast Forward - pressed"); break;
					case MediaControls.CT_Mute: console.log("Mute - pressed"); break;
				}
			}
			onControlClicked: (type) => {
				switch (type) {
					case MediaControls.CT_Play: console.log("Play - click"); break;
					case MediaControls.CT_Pause: console.log("Pause - click"); break;
					case MediaControls.CT_Stop: console.log("Stop - click"); break;
					case MediaControls.CT_Prev: console.log("Prev - click"); break;
					case MediaControls.CT_Next: console.log("Next - click"); break;
					case MediaControls.CT_Equalizer: console.log("Equalizer - click"); break;
					case MediaControls.CT_Rewind: console.log("Rewind - click"); break;
					case MediaControls.CT_FastForward: console.log("Fast Forward - click"); break;
					case MediaControls.CT_Mute: console.log("Mute - click"); break;
				}
			}
			onControlReleased: (type) => {
				switch (type) {
					case MediaControls.CT_Play: console.log("Play - released"); break;
					case MediaControls.CT_Pause: console.log("Pause - released"); break;
					case MediaControls.CT_Stop: console.log("Stop - released"); break;
					case MediaControls.CT_Prev: console.log("Prev - released"); break;
					case MediaControls.CT_Next: console.log("Next - released"); break;
					case MediaControls.CT_Equalizer: console.log("Equalizer - released"); break;
					case MediaControls.CT_Rewind: console.log("Rewind - released"); break;
					case MediaControls.CT_FastForward: console.log("Fast Forward - released"); break;
					case MediaControls.CT_Mute: console.log("Mute - released"); break;
				}
			}

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
			}

			Rectangle {
				anchors.fill: parent
				color: "transparent"
				border.color: appSettings.fontColor
				radius: 8
			}
		}*/

		TPFileViewer {
			mediaSource: "file:///home/guilhermef/Videos/Premiação - Dança Do Ventre Duo.mp4"
			previewSource: "video_preview"
			width: preferredWidth
			height: preferredHeight
		}
	}

	TPBalloonTip {
		id: textCopiedInfo
		height: 40
		message: qsTr("Text copied to the clipboard")
		button1Text: ""
		button2Text: ""
		parentPage: homePage
	}

	function showTextCopiedMessage(): void {
		textCopiedInfo.showTimed(3000, 0);
	}

	TPBalloonTip {
		id: generalMessagesPopup
		parentPage: homePage
		button1Text: ""
		button2Text: ""
	}

	function displayResultMessage(title: string, message: string, img_src: string, msecs: int): void {
		generalMessagesPopup.title = title;
		generalMessagesPopup.message = message;
		generalMessagesPopup.imageSource = img_src;
		if (msecs > 0)
			generalMessagesPopup.showTimed(msecs, 0);
		else
			generalMessagesPopup.show(0);
	}

	property PasswordDialog passwdDlg: null
	function showPasswordDialog(title: string, message: string): void {
		if (passwdDlg === null) {
			function createPasswordDialog() {
				let component = Qt.createComponent("qrc:/qml/Dialogs/PasswordDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					passwdDlg = component.createObject(contentItem, { parentPage: homePage, title: title, message: message });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createPasswordDialog();
		}
		passwdDlg.show(-1);
	}

	function canChangeSetMode(exercise_number: int, exercise_idx: int, set_number: int) : bool {
		return false;
	}

	function mesosViewIndex(): int {
		return 0;
	}

	function setMesosViewIndex(index: int) {
		return;
	}
}
