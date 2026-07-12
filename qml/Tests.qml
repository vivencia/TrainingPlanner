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

	TPPage {
		id: homePage
		objectName: "homePage"
		imageSource: ":/images/backgrounds/backimage-home.jpg"
		anchors.fill: parent
		signal mesosViewChanged(bool own_mesos);

		property MesoManager mesoManager: null

		Connections {
			target: ItemManager
			function onCppDataForQMLReady() : void {
			}
		}

		Loader {
			active: homePage.mesoManager !== null


		sourceComponent: TPPageMenu {
			id: _control
			objectName: "pageMenuTest"
			parentPage: homePage

			entriesList: [
				{ "label": qsTr("Send to client"), "image": "forward_", "btn_id": MesoManager.OPTION_SEND_TO_CLIENT,
							"enabled": enabledCondition(MesoManager.OPTION_SEND_TO_CLIENT), "visible": true },
				{ "label": qsTr("Save as"), "image": "download_", "btn_id": MesoManager.OPTION_SAVE_AS,
							"enabled": enabledCondition(MesoManager.OPTION_SAVE_AS), "visible": true },
				{ "label": qsTr("Send to"), "image": "attach_", "btn_id": MesoManager.OPTION_SEND_TO,
							"enabled": enabledCondition(MesoManager.OPTION_SEND_TO), "visible": true },
				{ "label": qsTr("Share"), "image": "share_", "btn_id": MesoManager.OPTION_SHARE,
							"enabled": enabledCondition(MesoManager.OPTION_SHARE), "visible": Qt.platform.os === "android" },
				{ "label": qsTr("Exercises Planner"), "image": "meso-splitplanner.png", btn_id: MesoManager.OPTION_EXERCISES_PLANNER,
							"enabled": enabledCondition(MesoManager.OPTION_EXERCISES_PLANNER), "visible": showIndicator},
			]

			Component.onCompleted: tpOpen();
			onMenuEntrySelected: (btn_id) => {
				switch (btn_id) {
				case MesoManager.OPTION_SEND_TO_CLIENT: homePage.mesoManager.sendMesocycleFileToClient(); break;
				case MesoManager.OPTION_SAVE_AS: homePage.mesoManager.mesoFileOperations().doFileOperation(FileOperations.OT_Download); break;
				case MesoManager.OPTION_SEND_TO: homePage.mesoManager.mesoFileOperations().doFileOperation(FileOperations.OT_Forward); break;
				case MesoManager.OPTION_SHARE: homePage.mesoManager.mesoFileOperations().doFileOperation(FileOperations.OT_Share); break;
				case MesoManager.OPTION_EXERCISES_PLANNE: homePage.mesoManager.getExercisesPlannerPage(); break;
				}
			}

			function enabledCondition(menu_entry: int): bool {
				return true;
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

	function openDialog(): void {
		//dlg.open();
	}
}
