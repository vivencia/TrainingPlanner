function showInExMenu(page, bImportVisible) {
	if (inexportMenu === null) {
		var inexportMenuComponent = Qt.createComponent("TPFloatingMenuBar.qml");
		inexportMenu = inexportMenuComponent.createObject(page, {});
		if (bImportVisible)
			inexportMenu.addEntry(qsTr("Import"), "import.png", 0);
		inexportMenu.addEntry(qsTr("Export"), "save-day.png", 1);
		if (Qt.platform.os === "android")
			inexportMenu.addEntry(qsTr("Share"), "export.png", 2);
		inexportMenu.menuEntrySelected.connect(selectedMenuOption);
	}
	inexportMenu.show(btnImExport, 0);
}

function selectedMenuOption(menuid) {
	switch (menuid) {
		case 0: mainwindow.chooseFileToImport(); break;
		case 1: exportTypeTip.init(false); break;
		case 2: exportTypeTip.init(true); break;
	}
}
