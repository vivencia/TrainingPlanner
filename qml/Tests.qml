pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Pdf

import TpQml
import TpQml.Pages
import TpQml.Widgets
import TpQml.Exercises
import TpQml.Dialogs

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

		TPFileViewer {
			mediaSource: "/home/guilhermef/Documents/Atendimento_CIP_35.001.003.26.1170764.pdf"
			//mediaSource: "/home/guilhermef/Videos/2026-crivania3-30fps.mp4"
			//mediaSource: ""
			canAddFile: true
			//mediaSource: "/home/guilhermef/Pictures/CNH Rozângela Barbosa Fortunato.png"
			//mediaSource: "/home/guilhermef/Documents/programa 2.txt"
			//mediaSource: "/home/guilhermef/.local/share/Vivencia Software/TrainingPlanner/1759256421787/chats/1759170252407/Ganho de capital 2025.pdf"
			x: 50
			y: 50
		}
	}

	Loader {
		id: openDialogLoader
		asynchronous: true
		active: false

//public:
		property int fileType

//private:
		property TPFileDialog _file_dialog

		sourceComponent: TPFileDialog {
			saveDialog: false
			fileType: openDialogLoader.fileType

			onDialogClosed: (result) => {
				mainwindow.fileDialogClosed(result === 0 ? AppUtils.getCorrectPath(currentFile) : "");
				openDialogLoader.active = false;
			}
			Component.onCompleted: openDialogLoader._file_dialog = this;
		}

		onLoaded: _file_dialog.open();
	}
	function chooseFileToOpen(filetype: int): void {
		openDialogLoader.fileType = filetype;
		openDialogLoader.active = true;
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
