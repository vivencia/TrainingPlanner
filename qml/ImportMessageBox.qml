import QtQuick

TPBalloonTip {
	id: msgbox
	imageSource: "qrc:/images/"+AppSettings.iconFolder+"import.png"
	title: qsTr("Attempt to import the file?")
	message: importFile
	button1Text: qsTr("Yes")
	button2Text: qsTr("No");

	property string importFile

	onButton1Clicked: {
		const result = appDB.importFromFile(importFile);
		mainwindow.displayResultMessage(result);
	}

	function init(file: string) {
		importFile = file;
		msgbox.show(-1);
	}
}

