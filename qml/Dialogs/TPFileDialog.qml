import QtQuick
import QtQuick.Dialogs

import TpQml

FileDialog {
	fileMode: saveDialog ? FileDialog.SaveFile : FileDialog.OpenFile
	options: saveDialog ? FileDialog.ReadOnly : 0
	nameFilters: AppUtils.fileExtension(fileType, false, true)
	currentFolder: AppUtils.standardPathForFileType(fileType)
	selectedFile: saveDialog ? suggestedName: ""

	property bool saveDialog: false
	property int fileType: AppUtils.FT_ANY_TYPE
	property string suggestedName

	signal dialogClosed(bool accepted);

	onAccepted: dialogClosed(true);
	onRejected: dialogClosed(false);
}
