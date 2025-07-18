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

	property int contentType
	onAccepted: mainwindow.openFileChosen(appUtils.getCorrectPath(currentFile), contentType);
	onRejected: mainwindow.openFileRejected("");
}
