import QtQuick

TPBalloonTip {
	imageSource: "qrc:/images/"+AppSettings.iconFolder+"import.png"
	title: qsTr("Attempt to import the file?")
	message: importFile
	button1Text: qsTr("Yes")
	button2Text: qsTr("No");

	property string importFile

	TPBalloonTip {
		id: afterImportTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"import.png"
		button1Text: "OK"
	}

	onButton1Clicked: {
		const result = appDB.importFromFile(importFile);
		displayResultMessage(result, importFile);
	}

	function displayResultMessage(result: int, filename: string) {
		var message;
		switch (result)
		{
			case  0: message = qsTr("Import was successfull"); break;
			case -1: message = qsTr("Failed to open file"); break;
			case -2: message = qsTr("File type not recognized"); break;
			case -3: message = qsTr("File is formatted wrongly or is corrupted"); break;
		}
		afterImportTip.title = header;
		afterImportTip.message = filename;
		afterImportTip.showTimed(5000, 0);
	}

	function init(file: string) {
		importFile = file;
		show(-1);
	}
}

