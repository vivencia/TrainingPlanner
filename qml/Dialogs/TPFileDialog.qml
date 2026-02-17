import QtQuick
import QtQuick.Dialogs
import QtCore

FileDialog {
	fileMode: saveDialog ? FileDialog.SaveFile : FileDialog.OpenFile
	defaultSuffix: "txt"
	options: saveDialog ? FileDialog.ReadOnly : 0
	nameFilters: filters

	signal dialogClosed(int result);

	property list<string> filters
	property string suggestedName
	property bool saveDialog: false
	property bool chooseDialog: true
	property bool includeTextFilter: false
	property bool includeDocFilesFilter: false
	property bool includePDFFilter: false
	property bool includeVideoFilter: false
	property bool includeImageFilter: false
	property bool includeAllFilesFilter: false

	function show() {
		if (!saveDialog && suggestedName !== "")
			selectedFile = suggestedName;
		filters.length = 0;
		currentFolder = StandardPaths.writableLocation(StandardPaths.DocumentsLocation);
		if (includeAllFilesFilter)
			filters.push(qsTr("Any") + " (*.*)");
		else {
			if (includeTextFilter)
				filters.push(qsTr("Training Planner's files") + " (*.txt)");
			if (includeDocFilesFilter)
				filters.push(qsTr("Documents") + " (*.doc *.docx *.odt)");
			if (includePDFFilter)
				filters.push(qsTr("PDF Files") + " (*.pdf)");
			if (includeVideoFilter) {
				currentFolder = StandardPaths.writableLocation(StandardPaths.MoviesLocation);
				filters.push(qsTr("Videos") + " (*.mp4 *.mkv)");
			}
			if (includeImageFilter) {
				currentFolder = StandardPaths.writableLocation(StandardPaths.PicturesLocation);
				filters.push(qsTr("Images") + " (*.jpg *.jpeg *.png)");
			}
		}
		open();
	}

	onAccepted: dialogClosed(0);
	onRejected: dialogClosed(1);
}
