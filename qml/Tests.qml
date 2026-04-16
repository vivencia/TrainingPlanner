import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Pdf

import TpQml
import TpQml.Pages
import TpQml.Widgets
import TpQml.Exercises

ApplicationWindow {
	id: mainwindow
	visible: true
	title: "TraininPlanner Tests"
	objectName: "mainwindow"
	width: AppSettings.windowWidth
	height: AppSettings.windowHeight
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint : Qt.Window | Qt.CustomizeWindowHint & ~Qt.WindowMaximizeButtonHint

	signal saveFileChosen(filepath: string);
	signal generalMessagesPopupClicked(button: int);
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
			height: AppSettings.itemDefaultHeight
			width: homePage.width * 0.8
			availableControls: [ MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Prev, MediaControls.CT_Next,
				MediaControls.CT_Equalizer, MediaControls.CT_Rewind, MediaControls.CT_FastForward, MediaControls.CT_VolumeUp,
				MediaControls.CT_VolumeDown, MediaControls.CT_Mute ]
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
					case MediaControls.CT_VolumeUp: console.log("Volume Up - pressed"); break;
					case MediaControls.CT_VolumeDown: console.log("Volume Down - pressed"); break;
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
					case MediaControls.CT_VolumeUp: console.log("Volume Up - click"); break;
					case MediaControls.CT_VolumeDown: console.log("Volume Down - çlick"); break;
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
					case MediaControls.CT_VolumeUp: console.log("Volume Up - released"); break;
					case MediaControls.CT_VolumeDown: console.log("Volume Down - released"); break;
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
				border.color: AppSettings.fontColor
				radius: 8
			}
		}*/

		/*TPFileViewer {
			//mediaSource: "/home/guilhermef/Documents/Atendimento_CIP_35.001.003.26.1170764.pdf"
			//mediaSource: "/home/guilhermef/Videos/Premiação - Dança Cigana Solo 2.mp4"
			//mediaSource: "/home/guilhermef/Pictures/CNH Rozângela Barbosa Fortunato.png"
			mediaSource: "/home/guilhermef/Documents/programa 2.txt"
			width: 300
			height: 300
			x: 0
			y: 0
		}*/

		FileOperations {
			id: fileOps
			fileType: AppUtils.FT_TP_PROGRAM
			mesoIdx: 0
		}

		TPPageMenu {
			parentPage: homePage
			entriesListModel: ListModel {
				ListElement { label: qsTr("Send to client"); image: "download_"; btn_id: TPFileOps.OT_Custom_1; }
				ListElement { label: qsTr("Save as"); image: "download_"; btn_id: TPFileOps.OT_Download; }
				ListElement { label: qsTr("Send to"); image: "attach_"; btn_id: TPFileOps.OT_Forward; }
				ListElement { label: qsTr("Share"); image: "share_"; btn_id: TPFileOps.OT_Share; }
				ListElement { label: qsTr("Exercises Planner"); image: "meso-splitplanner.png"; btn_id: TPFileOps.OT_Custom_2; }
			}
			entry_enabled: [  true, true, true, Qt.platform.os === "android", true ]

			onMenuEntrySelected: (btn_id) => {
				switch (btn_id) {
				case TPFileOps.OT_Custom_1: console.log("mesoPage.mesoManager.sendMesocycleFileToClient()"); break;
				case TPFileOps.OT_Custom_2: console.log("mesoPage.mesoManager.getExercisesPlannerPage()"); break;
				default: console.log("fileOps.doFileOperation(btn_id)");
				}
			}
		}
	}

	TPBalloonTip {
		id: generalMessagesPopup
		parentPage: homePage
	}

	function showAppMainMessageDialog(pos: int, title: string, message: string, img_src: string, msecs: int, button1Text: string,
																											button2Text: string): void {
		generalMessagesPopup.title = title;
		generalMessagesPopup.message = message;
		generalMessagesPopup.imageSource = img_src;
		generalMessagesPopup.button1Text = button1Text;
		generalMessagesPopup.button2Text = button2Text;
		if (button1Text !== "") {
			generalMessagesPopup.button1Clicked.connect(function() { generalMessagesPopupClicked(1); });
			generalMessagesPopup.closeActionExeced.connect(function() { generalMessagesPopupClicked(0); });
		}
		if (button2Text !== "")
			generalMessagesPopup.button2Clicked.connect(function() { generalMessagesPopupClicked(2); });
		if (msecs > 0)
			generalMessagesPopup.showTimed(msecs, pos);
		else
			generalMessagesPopup.showInWindow(pos);
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
