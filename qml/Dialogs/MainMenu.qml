import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Drawer {
	id: drawer
	height: mainwindow.height
	implicitWidth: mainwindow.width * 0.6
	spacing: 0
	padding: 0
	edge: Qt.LeftEdge
	opacity: 0.8

	property var buttonComponent: null

	background: Rectangle {
		id: backgrundRec
		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
			GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
			GradientStop { position: 0.50; color: AppSettings.primaryColor; }
			GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
		}
		opacity: 0.8
	}

	contentItem {
		Keys.onPressed: (event) => {
			if (event.key === mainwindow.backKey) {
				event.accepted = true;
				close();
			}
		}
	}

	TPButton {
		id: btnExit
		text: qsTr("Exit")
		imageSource: "application-exit.png"
		leftAlign: false
		rounded: false

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		onClicked: appDB.exitApp();
	}

	ColumnLayout {
		id: drawerLayout
		spacing: 5
		opacity: parent.opacity

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			bottom: btnExit.top
			leftMargin: 5
			rightMargin: 5
			topMargin: 10
			bottomMargin: 5
		}

		Rectangle {
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignCenter
			height: 230
			color: "transparent"

			Image {
				id: imgLogo
				source: "qrc:/images/app_icon.png"
				fillMode: Image.PreserveAspectFit
				height: 150
				width: 150

				anchors {
					left: parent.left
					right: parent.right
					top: parent.top
					bottomMargin: 10
				}
			}

			Label {
				text: "TrainingPlanner by VivenciaSoftware - " + AppSettings.appVersion
				wrapMode: Text.WordWrap
				font.bold: true
				font.pointSize: AppSettings.fontSizeText
				horizontalAlignment: Text.AlignHCenter
				color: AppSettings.fontColor

				anchors {
					left: parent.left
					right: parent.right
					top: imgLogo.bottom
					topMargin: 20
				}
			}
		}

		Rectangle {
			height: 3
			color: AppSettings.fontColor
			Layout.topMargin: 10
			Layout.fillWidth: true
		}

		TPButton {
			id: btnSettingsExDB
			Layout.fillWidth: true
			text: qsTr("Exercises Database")

			enabled: { // Force the binding to re-evaluate so that the title check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.title === "Exercises Page"; })
			}

			onClicked: {
				appDB.openExercisesListPage(false);
				close();
			}
		}

		TPButton {
			id: btnSettings
			text: qsTr("Settings")
			Layout.fillWidth: true

			enabled: { // Force the binding to re-evaluate so that the title check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.objectName === "configurationPage"; })
			}
			onClicked: {
				pageDeActivated_main(stackView.currentItem);
				stackView.push("../Pages/ConfigurationPage.qml");
				pageDeActivated_main(stackView.currentItem);
				close();
			}
		}

		TPButton {
			id: btnUser
			text: qsTr("Profile")
			Layout.fillWidth: true

			enabled: { // Force the binding to re-evaluate so that the title check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.objectName === "configurationPage"; })
			}
			onClicked: {
				pageDeActivated_main(stackView.currentItem);
				stackView.push("../Pages/ConfigurationPage.qml", { startPageIndex: 1 });
				pageDeActivated_main(stackView.currentItem);
				close();
			}
		}

		TPButton {
			id: btnBackup
			text: qsTr("Backup/Restore")
			Layout.fillWidth: true

			enabled: { // Force the binding to re-evaluate so that the title check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.objectName === "backupPage"; })
			}
			onClicked: {
				pageDeActivated_main(stackView.currentItem);
				stackView.push("../Pages/BackupPage.qml");
				pageDeActivated_main(stackView.currentItem);
				close();
			}
		}

		Rectangle {
			height: 3
			color: AppSettings.fontColor
			Layout.fillWidth: true
			Layout.bottomMargin: 10
		}

		Item { // spacer item
			Layout.fillWidth: true
			Layout.fillHeight: true
		}
	} //ColumnLayout

	function createShortCut(label: string, object: Item, clickid: int) {
		if (!buttonComponent)
			buttonComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPButton.qml", Qt.Asynchronous);

		function finishCreation() {
			var button = buttonComponent.createObject(drawerLayout, { text: label, clickId: clickid, "Layout.fillWidth": true });
			button.clicked.connect(appDB.openMainMenuShortCut);
			appDB.addMainMenuShortCutEntry(button);
		}

		if (buttonComponent.status === Component.Ready)
			finishCreation();
		else
			buttonComponent.statusChanged.connect(finishCreation);
	}
} //Drawer
