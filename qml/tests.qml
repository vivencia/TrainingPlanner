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
			lstWorkoutExercises.exercisesModel = itemManager.workoutModel();
			lstWorkoutExercises.currentIndex = itemManager.workoutModel().workingExercise;
		}
	}

	TPPage {
		id: homePage
		objectName: "homePage"
		anchors.fill: parent

		property MesocyclesModel mesoModel: null
		signal mesosViewChanged(bool own_mesos);

		WorkoutOrSplitExercisesList {
			id: lstWorkoutExercises
			pageManager: homePage
			anchors.fill: parent
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
}
