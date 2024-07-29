import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import "../"
import "../TPWidgets"

Page {
	id: backupPage
	objectName: "backupPage"

	property bool bCanRestoreMeso: false
	property bool bCanRestoreMesoSplit: false
	property bool bCanRestoreMesoCal: false
	property bool bCanRestoreTraining: false
	property bool bCanRestoreExercises: false
	property bool bCanWriteToBackupFolder: false
	property int opResult

	property int backupCount: 5
	property var backupFiles: [1,1,1,1,1]
	property bool bCanBackup: backupCount > 0
	property int restoreCount: 0
	property var restoreFiles: [0,0,0,0,0]
	property bool bCanRestore: restoreCount > 0
	property bool bNeedCheck: false

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
		imageSource: "backup.png"
		parentPage: backupPage

		onButton1Clicked: appDB.restartApp();

		function init(msg: string) {
			message = msg;
			showTimed(5000, 0);
		}

		function initButton(msg: string) {
			message = msg;
			button1Text = qsTr("Restart now");
			showTimed(5000, 0);
		}
	}

	onOpResultChanged: {
		var msg;
		switch (opResult)
		{
			case 1: msg = qsTr("Selected filed to backup successfully backed up"); break;
			case 2: msg = qsTr("Failed to backup selected files"); break;
			case 4: msg = qsTr("Failed to restore database from backup"); break;
			case 3:
				msg = qsTr("Database files successfully restored. You need to restart the app");
				opFinished.initButton(msg);
				return;
		}
		opFinished.init(msg);
	}

	FolderDialog {
		id: folderDialog;
		title: qsTr("Backup Destination Folder");
		currentFolder: AppSettings.backupFolder

		onAccepted: {
			bNeedCheck = true;
			AppSettings.backupFolder = runCmd.getCorrectPath(currentFolder);
			close();
		}
	}

	ScrollView {
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight

		ColumnLayout {
			id: colMain
			anchors.fill: parent
			spacing: 5

			Label {
				text: qsTr("Save backup to folder: ")
				color: AppSettings.fontColor
				font.bold: true
				font.pointSize: AppSettings.fontSize
				Layout.leftMargin: 5
			}

			TPTextInput {
				id: txtBackupLocation
				text: AppSettings.backupFolder
				width: backupPage.width - 80
				Layout.leftMargin: 10
				Layout.minimumWidth: width
				Layout.maximumWidth: width

				TPRoundButton {
					id: btnChooseBackupFolder
					imageName: "folder-backup.png"
					width: 40
					height: 40
					anchors.verticalCenter: parent.verticalCenter
					anchors.left: parent.right
					onClicked: folderDialog.open();
				}

				onTextChanged: {
					if (bNeedCheck)
						appDB.verifyBackupPageProperties(backupPage);
				}
			}

			TPGroupBox {
				text: qsTr("What to backup")
				Layout.alignment: Qt.AlignCenter
				Layout.bottomMargin: 20
				Layout.leftMargin: 10
				Layout.fillWidth: true
				Layout.maximumWidth: backupPage.width - 20

				ColumnLayout {
					anchors.fill: parent
					spacing: 0

					TPCheckBox {
						text: qsTr("Mesocycles")
						enabled: bCanWriteToBackupFolder
						checked: true
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++backupCount
							else --backupCount;
						}
						onCheckedChanged: backupFiles[0] = checked ? 1 : 0;
					}
					TPCheckBox {
						text: qsTr("Mesocycles splits")
						enabled: bCanWriteToBackupFolder
						checked: true
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++backupCount
							else --backupCount;
						}
						onCheckedChanged: backupFiles[1] = checked ? 1 : 0;
					}
					TPCheckBox {
						text: qsTr("Mesocycles calendar")
						enabled: bCanWriteToBackupFolder
						checked: true
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++backupCount
							else --backupCount;
						}
						onCheckedChanged: backupFiles[2] = checked ? 1 : 0;
					}
					TPCheckBox {
						text: qsTr("Workouts information")
						enabled: bCanWriteToBackupFolder
						checked: true
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++backupCount
							else --backupCount;
						}
						onCheckedChanged: backupFiles[3] = checked ? 1 : 0;
					}
					TPCheckBox {
						text: qsTr("Exercises database")
						enabled: bCanWriteToBackupFolder
						checked: true
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++backupCount
							else --backupCount;
						}
						onCheckedChanged: backupFiles[4] = checked ? 1 : 0;
					}
				} //ColumnLayout
			} //TPGroupBox

			TPButton {
				id: btnBackup
				text: qsTr("Backup")
				enabled: bCanWriteToBackupFolder && bCanBackup
				Layout.alignment: Qt.AlignCenter

				onClicked: appDB.copyDBFilesToUserDir(backupPage, txtBackupLocation.text, backupFiles);
			}

			Rectangle {
				height: 3
				color: AppSettings.fontColor
				Layout.fillWidth: true
				Layout.topMargin: 10
				Layout.bottomMargin: 10
			}

			Label {
				text: qsTr("Restore from folder: ")
				color: AppSettings.fontColor
				font.bold: true
				font.pointSize: AppSettings.fontSize
				Layout.leftMargin: 5
				Layout.topMargin: 20
				Layout.bottomMargin: 20
			}

			TPTextInput {
				id: txtRestoreLocation
				text: AppSettings.backupFolder
				width: backupPage.width - 60
				Layout.leftMargin: 10
				Layout.minimumWidth: width
				Layout.maximumWidth: width

				TPRoundButton {
					id: btnChooseFolder
					imageName: "folder-backup.png"
					width: 40
					height: 40
					anchors.verticalCenter: parent.verticalCenter
					anchors.left: parent.right
					onClicked: folderDialog.open();
				}

				onTextChanged: {
					if (bNeedCheck)
						appDB.verifyBackupPageProperties(backupPage);
				}
			}

			TPGroupBox {
				text: qsTr("What to restore")
				Layout.alignment: Qt.AlignCenter
				Layout.bottomMargin: 20
				Layout.leftMargin: 10
				Layout.fillWidth: true
				Layout.maximumWidth: backupPage.width - 20

				ColumnLayout {
					anchors.fill: parent
					spacing: 0

					TPCheckBox {
						id: chkMeso
						text: qsTr("Mesocycles")
						enabled: bCanRestoreMeso
						checked: bCanRestoreMeso
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++restoreCount
							else --restoreCount;
						}
						onCheckedChanged: restoreFiles[0] = checked ? 1 : 0;
					}
					TPCheckBox {
						id: chkMesoSplit
						text: qsTr("Mesocycles splits")
						enabled: bCanRestoreMesoSplit
						checked: bCanRestoreMesoSplit
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++restoreCount
							else --restoreCount;
						}
						onCheckedChanged: restoreFiles[1] = checked ? 1 : 0;
					}
					TPCheckBox {
						id: chkMesoCal
						text: qsTr("Mesocycles calendar")
						enabled: bCanRestoreMesoCal
						checked: bCanRestoreMesoCal
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++restoreCount
							else --restoreCount;
						}
						onCheckedChanged: restoreFiles[2] = checked ? 1 : 0;
					}
					TPCheckBox {
						id: chkTraining
						text: qsTr("Workouts information")
						enabled: bCanRestoreTraining
						checked: bCanRestoreTraining
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++restoreCount
							else --restoreCount;
						}
						onCheckedChanged: restoreFiles[3] = checked ? 1 : 0;
					}
					TPCheckBox {
						id: chkExercises
						text: qsTr("Exercises database")
						enabled: bCanRestoreExercises
						checked: bCanRestoreExercises
						Layout.leftMargin: 10

						onClicked: {
							if (checked ) ++restoreCount
							else --restoreCount;
						}
						onCheckedChanged: restoreFiles[4] = checked ? 1 : 0;
					}
				} //ColumnLayout
			}//TPGroupBox

			TPButton {
				id: btnRestore
				text: qsTr("Restore")
				enabled: bCanRestore
				Layout.alignment: Qt.AlignCenter

				onClicked: appDB.copyFileToAppDataDir(backupPage, txtBackupLocation.text, restoreFiles);
			}

			Item { // spacer item
				Layout.fillWidth: true
				Layout.fillHeight: true
			}
		} //ColumLayout
	} //ScrollView

	Component.onCompleted: appDB.verifyBackupPageProperties(backupPage);
} //Page
