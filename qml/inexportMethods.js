function showInExMenu(page, bImportVisible) {
	if (imexportMenu === null) {
		var imexportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
		imexportMenu = imexportMenuComponent.createObject(page, {});
		if (bImportVisible)
			imexportMenu.addEntry(qsTr("Import"), "import.png", 0);
		imexportMenu.addEntry(qsTr("Export"), "save-day.png", 1);
		if (Qt.platform.os === "android")
			imexportMenu.addEntry(qsTr("Share"), "export.png", 2);
		imexportMenu.menuEntrySelected.connect(selectedMenuOption);
	}
	imexportMenu.enableMenuEntry(1, bExportEnabled);
	if (Qt.platform.os === "android")
		imexportMenu.enableMenuEntry(2, bExportEnabled);
	imexportMenu.show(btnImExport, 0);
}

function selectedMenuOption(menuid) {
	switch (menuid) {
		case 0: mainwindow.chooseFileToImport(); break;
		case 1: exportTypeTip.init(false); break;
		case 2: exportTypeTip.init(true); break;
	}
}
