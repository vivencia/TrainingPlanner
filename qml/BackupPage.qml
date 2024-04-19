import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Page {
	id: backupPage
	objectName: "backupPage"

	property bool bCanRestoreMeso: false
	property bool bCanRestoreMesoSplit: false
	property bool bCanRestoreMesoCal: false
	property bool bCanRestoreExercises: false
	property bool bCanRestoreTraining: false
	property bool bCanWriteToBackupFolder: false
	property bool bCanRestore: restoreCount > 0

	property int restoreCount: 0

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

	TPBalloonTip {
		id: opFinished
		imageSource: "qrc:/images/"+lightIconFolder+"settings.png"

		function init(msg: string) {
			message = msg;
			showTimed(3000, 0);
		}
	}

	FolderDialog {
		id: folderDialog;
		title: qsTr("Backup Destination Folder");
		currentFolder: AppSettings.backupFolder

		onAccepted: {
			console.log(runCmd.getCorrectPath(currentFolder));
			AppSettings.backupFolder = runCmd.getCorrectPath(currentFolder);
			close();
		}
	}

	ColumnLayout {
		id: colMain
		anchors.fill: parent
		spacing: 5

		Label {
			text: qsTr("BACKUP DATA")
			color: "white"
			font.bold: true
			font.pixelSize: AppSettings.fontSize
			Layout.alignment: Qt.AlignCenter
			Layout.topMargin: 20
			Layout.bottomMargin: 20
		}

		Label {
			text: qsTr("Save backup to folder: ")
			color: "white"
			font.bold: true
			font.pixelSize: AppSettings.fontSize
			Layout.leftMargin: 5
		}

		TPTextInput {
			id: txtBackupLocation
			text: AppSettings.backupFolder
			width: backupPage.width - 60
			Layout.leftMargin: 10
			Layout.minimumWidth: width
			Layout.maximumWidth: width

			RoundButton {
				id: btnChooseFolder
				icon.source: "qrc:/images/"+darkIconFolder+"time.png"
				width: 40
				height: 40

				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.right
				onClicked: folderDialog.open();
			}

			onTextChanged: appDB.verifyBackupPageProperties(backupPage);
		}

		TPButton {
			id: btnBackup
			text: qsTr("Backup")
			enabled: bCanWriteToBackupFolder
			Layout.alignment: Qt.AlignCenter

			onClicked: runCmd.copyFileToAppDataDir(txtBackupLocation.text);
		}

		Rectangle {
			height: 3
			color: "white"
			Layout.fillWidth: true
			Layout.topMargin: 10
			Layout.bottomMargin: 10
		}

		TPGroupBox {
			text: qsTr("RESTORE DATA")
			Layout.alignment: Qt.AlignCenter
			Layout.bottomMargin: 20
			Layout.leftMargin: 10
			Layout.fillWidth: true
			Layout.maximumWidth: backupPage.width - 20

			ColumnLayout {
				anchors.fill: parent
				spacing: 0

				Label {
					text: qsTr("Restore from folder: ")
					color: "white"
					font.bold: true
					font.pixelSize: AppSettings.fontSize
					Layout.leftMargin: 5
				}

				TPTextInput {
					id: txtRestoreLocation
					text: AppSettings.backupFolder
					width: backupPage.width - 80
					Layout.leftMargin: 10
					Layout.minimumWidth: width
					Layout.maximumWidth: width

					RoundButton {
						id: btnChooseRestoreFolder
						icon.source: "qrc:/images/"+darkIconFolder+"time.png"
						width: 40
						height: 40
						anchors.verticalCenter: parent.verticalCenter
						anchors.left: parent.right
						onClicked: folderDialog.open();
					}

					onTextChanged: appDB.verifyBackupPageProperties(backupPage);
				}

				TPCheckBox {
					id: chkMeso
					text: qsTr("Mesocycles")
					enabled: bCanRestoreMeso
					checked: bCanRestoreMeso
					Layout.leftMargin: 10

					onCheckedChanged: {
						if (checked ) ++restoreCount
						else --restoreCount;
					}
				}
				TPCheckBox {
					id: chkMesoSplit
					text: qsTr("Mesocycles Splits")
					enabled: bCanRestoreMesoSplit
					checked: bCanRestoreMesoSplit
					Layout.leftMargin: 10

					onCheckedChanged: {
						if (checked ) ++restoreCount
						else --restoreCount;
					}
				}
				TPCheckBox {
					id: chkMesoCal
					text: qsTr("Mesocycles Calendar")
					enabled: bCanRestoreMesoCal
					checked: bCanRestoreMesoCal
					Layout.leftMargin: 10

					onCheckedChanged: {
						if (checked ) ++restoreCount
						else --restoreCount;
					}
				}
				TPCheckBox {
					id: chkExercises
					text: qsTr("Exercises database")
					enabled: bCanRestoreExercises
					checked: bCanRestoreExercises
					Layout.leftMargin: 10

					onCheckedChanged: {
						if (checked ) ++restoreCount
						else --restoreCount;
					}
				}
				TPCheckBox {
					id: chkTraining
					text: qsTr("Workout days")
					enabled: bCanRestoreTraining
					checked: bCanRestoreTraining
					Layout.leftMargin: 10

					onCheckedChanged: {
						if (checked ) ++restoreCount
						else --restoreCount;
					}
				}
			} //ColumnLayout
		} //TPGroupBox

		TPButton {
			id: btnRestore
			text: qsTr("Restore")
			enabled: bCanRestore
			Layout.alignment: Qt.AlignCenter

			onClicked: runCmd.copyFileToAppDataDir(txtBackupLocation.text);
		}

		Item { // spacer item
			Layout.fillWidth: true
			Layout.fillHeight: true
		}
	} //ColumLayout
} //Page
