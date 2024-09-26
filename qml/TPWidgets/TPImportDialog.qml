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

	onAccepted: mainwindow.openFileChosen(appUtils.getCorrectPath(currentFile));
	onRejected: mainwindow.openFileRejected("");
}
