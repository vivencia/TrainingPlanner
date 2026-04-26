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

	signal saveFileChosen(filepath: string);
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
			width: 3000
			height: 3000
			x: 0
			y: 0
		}*/

		PasswordDialog {
			id: passwd
			request_id: 40
			title: "Root Password"
			message: "Give me your root password"
			parentPage: homePage
		}

		TPButton {
			text: "Show menu"
			autoSize: true
			anchors.right: txtDummy.left
			anchors.top: txtDummy.top
			onClicked: {
				//_control.showByWidget();
				menu.reference_widget = txtDummy;
				menu.showIndicator = false;
				menu.tpOpen();
				//balloon.show_position = Qt.AlignLeft
				//balloon.tpOpen();
			}
		}
		TPTextInput {
			id: txtDummy
			opacity: 1
			width: 200
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
		}
		TPButton {
			text: "Show page"
			autoSize: true
			anchors.left: txtDummy.right
			anchors.top: txtDummy.top
			onClicked: {
				ItemManager.AppPagesManager.openPopup(passwd, homePage);
				//_control.showByWidget();
				//menu.reference_widget = null;
				//menu.showIndicator = true;
				//menu.tpOpen();
				//balloon.show_position = Qt.AlignLeft
				//balloon.tpOpen();
			}
		}

		TPBalloonTip {
			id: balloon
			parentPage: homePage
			open_in_window: true
			_use_burst_transition: false
			message: "mgeprgpegpoeṕgégṕeǵ féwfpewkfṕew'fẃe,fẃe,f fed.fçdf,léw,f"
			title: "feḱwéw,ld,çç"
			button1Text: "fkweopfkpow"
			button2Text: "nvcxpvci"
		}

		FileOperations {
			id: fileOps
			fileType: AppUtils.FT_TP_PROGRAM
			mesoIdx: 0
		}

		TPPageMenu {
			id: menu
			parentPage: homePage
			showIndicator: true
			_use_burst_transition: true

			entriesList: [
				{ "label": qsTr("Send to client"), "image": "download_", "btn_id": TPFileOps.OT_Custom_1, "enabled": true },
				{ "label": qsTr("Save as"), "image": "download_", "btn_id": TPFileOps.OT_Download, "enabled": true },
				{ "label": qsTr("Send to"), "image": "attach_", "btn_id": TPFileOps.OT_Forward, "enabled": true },
				{ "label": qsTr("Share"), "image": "share_", "btn_id": TPFileOps.OT_Share, "enabled": Qt.platform.os === "android" },
				{ "label": qsTr("Exercises Planner"), "image": "meso-splitplanner.png", btn_id: TPFileOps.OT_Custom_2, "enabled": false },
			]

			onMenuEntrySelected: (btn_id) => {
				switch (btn_id) {
				case TPFileOps.OT_Custom_1: enableEntry(4, true); break;
				case TPFileOps.OT_Custom_2: enableEntry(0, false); break;
				default: console.log("fileOps.doFileOperation(btn_id)");
				}
			}
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
}
