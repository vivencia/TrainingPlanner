function showInExMenu(page) {
	if (inexportMenu === null) {
		var inexportMenuComponent = Qt.createComponent("TPFloatingMenuBar.qml");
		inexportMenu = inexportMenuComponent.createObject(page, {});
		inexportMenu.addEntry(qsTr("Import"), "import.png", 0);
		inexportMenu.addEntry(qsTr("Save"), "save-day.png", 1);
		if (Qt.platform.os === "android")
			inexportMenu.addEntry(qsTr("Export"), "export.png", 2);
		inexportMenu.menuEntrySelected.connect(selectedMenuOption);
	}
	inexportMenu.show(btnImExport, 0);
}

function selectedMenuOption(menuid) {
	switch (menuid) {
		case 0: mainwindow.chooseFileToImport(); break;
		case 1: exportTypeTip.init(true); break;
		case 2: exportTypeTip.init(false); break;
	}
}
