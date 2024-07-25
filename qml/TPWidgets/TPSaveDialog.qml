import QtQuick
import QtQuick.Dialogs
import QtCore

FileDialog {
	id: mainDialog
	title: qsTr("Choose a folder to save to")
	currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
	fileMode: FileDialog.SaveFile

	onAccepted: appDB.saveFileDialogClosed(currentFile, true);
	onRejected: appDB.saveFileDialogClosed(currentFile, false);

	function init(suggestedName: string) {
		currentFile = suggestedName;
		open();
	}
}
