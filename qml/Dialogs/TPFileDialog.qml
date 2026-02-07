import QtQuick
import QtQuick.Dialogs
import QtCore

FileDialog {
	fileMode: saveDialog ? FileDialog.SaveFile : FileDialog.OpenFile
	defaultSuffix: "txt"
	options: saveDialog ? FileDialog.ReadOnly : 0
	selectedFile: suggestedName

	signal dialogClosed(int result);

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
		nameFilters = 0;
		currentFolder = StandardPaths.writableLocation(StandardPaths.DocumentsLocation);
		if (includeAllFilesFilter)
			nameFilter.push(qsTr("Any file type") + " (*.*)");
		else {
			if (includeTextFilter)
				nameFilter.push(qsTr("Training Planner's files") + " (*.txt)");
			if (includeDocFilesFilter)
				nameFilter.push(qsTr("Documents") + " (*.doc *.docx *.odt)");
			if (includePDFFilter)
				nameFilter.push(qsTr("PDF Files") + " (*.pdf)");
			if (includeVideoFilter) {
				currentFolder = StandardPaths.writableLocation(StandardPaths.MoviesLocation);
				nameFilter.push(qsTr("Videos") + " (*.mp4 *.mkv)");
			}
			if (includeImageFilter) {
				currentFolder = StandardPaths.writableLocation(StandardPaths.PicturesLocation);
				nameFilter.push(qsTr("Images") + " (*.jpg *.jpeg *.png)");
			}
		}
		open();
	}

	onAccepted: dialogClosed(0);
	onRejected: dialogClosed(1);
}
