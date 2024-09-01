import QtQuick

TPBalloonTip {
	id: msgbox
	imageSource: "import.png"
	title: qsTr("Attempt to import the file?")
	message: importFileName
	button1Text: qsTr("Yes")
	button2Text: qsTr("No");

	property string importFile
	property string importFileName

	onButton1Clicked: {
		const result = appDB.importFromFile(importFile);
		mainwindow.displayResultMessage(result);
	}

	function init(file: string, name: string) {
		importFile = file;
		importFileName = name;
		msgbox.show(-1);
	}
}

