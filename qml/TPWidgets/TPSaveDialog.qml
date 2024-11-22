import QtQuick
import QtQuick.Dialogs
import QtCore

FileDialog {
	id: mainDialog
	title: qsTr("Choose a folder to save to")
	currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
	fileMode: FileDialog.SaveFile
	defaultSuffix: "txt"
	nameFilters: [qsTr("Text files") + " (*.txt)"]

	onAccepted: mainwindow.saveFileChosen(appUtils.getCorrectPath(currentFile));
	onRejected: mainwindow.saveFileRejected("");

	function init(suggestedName: string, bShare: bool, extraArg: string) {
		selectedFile = suggestedName;
		open();
	}
}
