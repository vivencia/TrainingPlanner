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
		msgImport.displayResultMessage(result, currentFile);
		close();
	}

	ImportMessageBox {
		id: msgImport
	}
}
