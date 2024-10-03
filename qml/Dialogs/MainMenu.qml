import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import com.vivenciasoftware.qmlcomponents

Drawer {
	id: drawer
	height: mainwindow.height
	implicitWidth: mainwindow.width * 0.6
	spacing: 0
	padding: 0
	edge: Qt.LeftEdge
	opacity: 0.8

	property var buttonComponent: null
	property QmlItemManager itemManager

	onOpened: {
		if (userModel.avatar(0) !== imgAvatar.source) {
			imgAvatar.source = userModel.avatar(0);
			imgAvatar.update();
		}
		if (userModel.userName(0) !== lblAvatar.text)
			lblAvatar.text = userModel.userName(0);
	}

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

		onClicked: osInterface.exitApp();
	}

	ColumnLayout {
		id: drawerLayout
		objectName: "drawerLayout"
		spacing: 5
		opacity: parent.opacity

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			bottom: btnExit.top
			leftMargin: 5
			rightMargin: 5
			topMargin: 0
			bottomMargin: 5
		}

		TPImage {
			id: imgLogo
			source: "app_icon"
			dropShadow: false
			width: 100
			height: 100
			Layout.alignment: Qt.AlignHCenter
			Layout.topMargin: 0
		}

		Label {
			text: "TrainingPlanner by VivenciaSoftware - " + AppSettings.appVersion
			wrapMode: Text.WordWrap
			font.bold: true
			font.pointSize: AppSettings.fontSizeText
			horizontalAlignment: Text.AlignHCenter
			color: AppSettings.fontColor
			Layout.fillWidth: true
			Layout.topMargin: 0
		}

		TPImage {
			id: imgAvatar
			dropShadow: true
			width: 100
			height: 100
			Layout.alignment: Qt.AlignHCenter
			Layout.topMargin: 0

			MouseArea {
				anchors.fill: parent
				enabled: { // Force the binding to re-evaluate so that the check is run each time the page changes.
					stackView.currentItem
					!stackView.find((item, index) => { return item.objectName === "configurationPage"; })
				}

				onClicked: {
					itemManager.getSettingsPage(1);
					close();
				}
			}
		}

		Label {
			id: lblAvatar
			wrapMode: Text.WordWrap
			font.bold: true
			font.pointSize: AppSettings.fontSizeText
			horizontalAlignment: Text.AlignHCenter
			color: AppSettings.fontColor
			Layout.fillWidth: true
			Layout.topMargin: 0
		}

		Rectangle {
			height: 3
			color: AppSettings.fontColor
			Layout.topMargin: 10
			Layout.fillWidth: true
		}

		TPButton {
			id: btnExercises
			Layout.fillWidth: true
			text: qsTr("Exercises Database")

			enabled: { // Force the binding to re-evaluate so that the objectName check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.objectName === "exercisesPage"; })
			}

			onClicked: {
				itemManager.getExercisesPage(false);
				close();
			}
		}

		TPButton {
			id: btnSettings
			text: qsTr("Settings")
			Layout.fillWidth: true

			enabled: { // Force the binding to re-evaluate so that the check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.objectName === "configurationPage"; })
			}

			onClicked: {
				itemManager.getSettingsPage(0);
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

	function createShortCut(label: string, page: Item, clickid: int) {
		if (!buttonComponent)
			buttonComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPButton.qml", Qt.Asynchronous);

		function finishCreation() {
			var button = buttonComponent.createObject(drawerLayout, { text: label, clickId: clickid, associatedItem: page, "Layout.fillWidth": true });
			button.clicked.connect(function () { itemManager.openMainMenuShortCut();} );
			itemManager.addMainMenuShortCutEntry(button);
		}

		if (buttonComponent.status === Component.Ready)
			finishCreation();
		else
			buttonComponent.statusChanged.connect(finishCreation);
	}
} //Drawer
