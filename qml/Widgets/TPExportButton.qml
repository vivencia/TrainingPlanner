pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Pages

TPButton {
	id: _control
	text: qsTr("Export")
	imageSource: "export.png"

//public:
	required property TPPage parentPage
	onClicked: exportMenuLoader.active = true;

	FileOperations {
		id: fileOps
		fileType: AppUtils.FT_TP_PROGRAM
	}

	Loader {
		id: exportMenuLoader
		asynchronous: true
		active: false

		property TPFloatingMenuBar _export_menu;
		property TPButton exportButton

		sourceComponent: TPFloatingMenuBar {
			parentPage: _control.parentPage
			titleHeader: qsTr("Export options")
			entriesList: [
				{ "label": qsTr("Save as"), "image": "download_", "id": TPFileOps.OT_Download, "visible": true },
				{ "label": qsTr("Send to"), "image": "attach_", "id": TPFileOps.OT_Forward, "visible": true },
				{ "label": qsTr("Share"), "image": "share_", "id": TPFileOps.OT_Share, "visible": Qt.platform.os === "android"},
			]
			onMenuEntrySelected: (id) => fileOps.doFileOperation(id);
			onClosed: exportMenuLoader.active = false;
			Component.onCompleted: exportMenuLoader._export_menu = this;
		}

		onLoaded: _export_menu.showByWidget(_control, Qt.AlignTop);
	}
}
