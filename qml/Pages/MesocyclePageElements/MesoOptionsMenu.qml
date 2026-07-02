import QtQuick

import TpQml
import TpQml.Widgets

TPPageMenu {
	id: _control

	entriesList: [
		{ "label": qsTr("Send to client"), "image": "forward_", "btn_id": TPFileOps.OT_Custom_1,
					"enabled": enabledCondition(MesoManager.OPTION_SEND_TO_CLIENT), "visible": true },
		{ "label": qsTr("Save as"), "image": "download_", "btn_id": TPFileOps.OT_Download,
					"enabled": enabledCondition(MesoManager.OPTION_SAVE_AS), "visible": true },
		{ "label": qsTr("Send to"), "image": "attach_", "btn_id": TPFileOps.OT_Forward,
					"enabled": enabledCondition(MesoManager.OPTION_SEND_TO), "visible": true },
		{ "label": qsTr("Share"), "image": "share_", "btn_id": TPFileOps.OT_Share,
					"enabled": enabledCondition(MesoManager.OPTION_SHARE), "visible": Qt.platform.os === "android" },
		{ "label": qsTr("Exercises Planner"), "image": "meso-splitplanner.png", btn_id: TPFileOps.OT_Custom_2,
					"enabled": enabledCondition(MesoManager.OPTION_EXERCISES_PLANNER), "visible": showIndicator},
	]

	required property MesoManager mesoManager

	onMenuEntrySelected: (btn_id) => {
		switch (btn_id) {
			case TPFileOps.OT_Custom_1: mesoManager.sendMesocycleFileToClient(); break;
			case TPFileOps.OT_Custom_2: mesoManager.getExercisesPlannerPage(); break;
			default: fileOps.doFileOperation(btn_id); break;
		}
	}

	Connections {
		target: _control.mesoManager
		function onCanExportChanged(): void {
			_control.enableEntryById(TPFileOps.OT_Download, _control.enabledCondition(MesoManager.OPTION_SAVE_AS));
			_control.enableEntryById(TPFileOps.OT_Forward, _control.enabledCondition(MesoManager.OPTION_SEND_TO));
			_control.enableEntryById(TPFileOps.OT_Share, _control.enabledCondition(MesoManager.OPTION_SHARE));
		}
		function onSplitOKChanged(): void {
			_control.enableEntryById(TPFileOps.OT_Custom_2, _control.enabledCondition(MesoManager.OPTION_EXERCISES_PLANNER));
		}
		function onCanSendToClientChanged(): void {
			_control.enableEntryById(TPFileOps.OT_Custom_1, _control.enabledCondition(MesoManager.OPTION_SEND_TO_CLIENT));
		}
	}

	FileOperations {
		id: fileOps
		fileType: AppUtils.FT_TP_PROGRAM
		mesoIdx: _control.mesoManager.mesoIdx
	}

	function enabledCondition(menu_entry: int): bool {
		switch (menu_entry) {
			case MesoManager.OPTION_SEND_TO_CLIENT: return mesoManager.canSendToClient;
			case MesoManager.OPTION_SAVE_AS:
			case MesoManager.OPTION_SEND_TO:
			case MesoManager.OPTION_SHARE: return mesoManager.canExport;
			case MesoManager.OPTION_EXERCISES_PLANNER: return mesoManager.splitOK;
		}
	}
}
