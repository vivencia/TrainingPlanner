pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Pdf
import QtQuick.Layouts

import TpQml
import TpQml.Pages
import TpQml.Widgets
import TpQml.Exercises
import TpQml.Dialogs
import TpQml.User

ApplicationWindow {
	id: mainwindow
	visible: true
	title: "TraininPlanner Tests"
	objectName: "mainwindow"
	width: AppSettings.windowWidth
	height: AppSettings.windowHeight
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint : Qt.Window | Qt.CustomizeWindowHint & ~Qt.WindowMaximizeButtonHint

	signal fileDialogClosed(filepath: string);
	signal tpFileOpenInquiryResult(do_import: bool);

	Connections {
		target: ItemManager
		function onCppDataForQMLReady() : void {
			//lstWorkoutExercises.exercisesModel = itemManager.workoutModel();
			//lstWorkoutExercises.currentIndex = itemManager.workoutModel().workingExercise;
		}
	}

	TPPage {
		id: homePage
		objectName: "homePage"
		imageSource: ":/images/backgrounds/backimage-home.jpg"
		anchors.fill: parent
		signal mesosViewChanged(bool own_mesos);


		ColumnLayout {
			anchors.fill: parent

			TPCoachesAndClientsList {
				id: usersList
				listClients: true
				listCoaches: true
				listConfirmed: true
				enabled: selectedUsers.length === 0
				Layout.fillWidth: true
				Layout.preferredHeight: 300
			}

			TPButton {
				text: "Info"
				autoSize: true
				onClicked: console.log(usersList.model.data(AppUserModel.USER_FIELD_NAME, 0, 0));
			}

			/*SendFileToDialog {
				id: dlg
				handle: -1
				message: "Test message"
			}*/
		}
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

	function openDialog(): void {
		//dlg.open();
	}
}
