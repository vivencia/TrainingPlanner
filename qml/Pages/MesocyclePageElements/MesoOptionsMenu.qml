import QtQuick

import TpQml
import TpQml.Widgets

TPPageMenu {
	id: _control

	entriesList: [
		{ "label": qsTr("Send to client"), "image": "download_", "btn_id": TPFileOps.OT_Custom_1,
																	"enabled": enabledCondition(MesoManager.OPTION_SEND_TO_CLIENT) },
		{ "label": qsTr("Save as"), "image": "download_", "btn_id": TPFileOps.OT_Download,
																	"enabled": enabledCondition(MesoManager.OPTION_SAVE_AS) },
		{ "label": qsTr("Send to"), "image": "attach_", "btn_id": TPFileOps.OT_Forward,
																	"enabled": enabledCondition(MesoManager.OPTION_SEND_TO) },
		{ "label": qsTr("Share"), "image": "share_", "btn_id": TPFileOps.OT_Share,
																	"enabled": enabledCondition(MesoManager.OPTION_SHARE) },
		{ "label": qsTr("Exercises Planner"), "image": "meso-splitplanner.png", btn_id: TPFileOps.OT_Custom_2,
																	"enabled": enabledCondition(MesoManager.OPTION_EXERCISES_PLANNER) },
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
			for (let i = 0; i <= 3; ++i)
				_control.enableEntry(i, _control.enabledCondition(i));
		}
		function onSplitOKChanged(): void {
			_control.enableEntry(MesoManager.OPTION_EXERCISES_PLANNER, _control.enabledCondition(4));
		}
	}

	FileOperations {
		id: fileOps
		fileType: AppUtils.FT_TP_PROGRAM
		mesoIdx: _control.mesoManager.mesoIdx
	}

	function enabledCondition(menu_entry: int): bool {
		switch (menu_entry) {
			case MesoManager.OPTION_SEND_TO_CLIENT: return !mesoManager.ownMeso && mesoManager.canExport;
			case MesoManager.OPTION_SAVE_AS:
			case MesoManager.OPTION_SEND_TO: return mesoManager.canExport;
			case MesoManager.OPTION_SHARE: return mesoManager.canExport && Qt.platform.os === "android";
			case MesoManager.OPTION_EXERCISES_PLANNER: return mesoManager.splitOK;
		}
	}
}
