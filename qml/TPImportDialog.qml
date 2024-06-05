import QtQuick
import QtQuick.Dialogs
import QtCore

FileDialog {
	id: mainDialog
	title: qsTr("Choose the file to import from")
	currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
	fileMode: FileDialog.OpenFile

	property int _opt
	property bool _bfancyFormat

	onAccepted: {
		//const result = appDB.importFromFile(currentFile);
		const result = appDB.importFromFile("/home/guilherme/Dokumente/tp/tday.tp");
		var message;
		switch (result)
		{
			case  0: message = qsTr("Import was successfull"); break;
			case -1: message = qsTr("Failed to open file"); break;
			case -2: message = qsTr("File type not recognized"); break;
			case -3: message = qsTr("File is formatted wrongly or is corrupted"); break;
		}
		afterImportTip.init(message, currentFile);
		close();
	}

	TPBalloonTip {
		id: afterImportTip
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"import.png"
		button1Text: "OK"

		function init(header: string, msg: string) {
			title = header;
			message = msg;
			showTimed(5000, 0);
		}
	}
}
