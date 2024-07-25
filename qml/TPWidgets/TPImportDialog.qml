import QtQuick
import QtQuick.Dialogs
import QtCore

FileDialog {
	id: mainDialog
	title: qsTr("Choose the file to import from")
	defaultSuffix: "txt"
	nameFilters: ["Training Planner's files (*.txt)"]
	currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
	fileMode: FileDialog.OpenFile

	property int _opt
	property bool _bfancyFormat

	onAccepted: {
		mainwindow.importExportFilename = currentFile;
		const result = appDB.parseFile(currentFile);
		mainwindow.displayResultMessage(result);
		close();
	}
}
