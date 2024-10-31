import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Drawer {
	id: drawer
	height: mainwindow.height
	implicitWidth: mainwindow.width * 0.6
	spacing: 0
	padding: 0
	edge: Qt.LeftEdge
	opacity: 0.8

	property var buttonComponent: null

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
			GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
			GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
			GradientStop { position: 0.50; color: appSettings.primaryColor; }
			GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
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

		TPLabel {
			text: "TrainingPlanner by VivenciaSoftware - " + appSettings.appVersion
			width: drawer.implicitWidth-10
			singleLine: false
			horizontalAlignment: Text.AlignHCenter
			Layout.minimumWidth: width
			Layout.maximumWidth: width
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

		TPLabel {
			id: lblAvatar
			horizontalAlignment: Text.AlignHCenter
			Layout.fillWidth: true
			Layout.topMargin: 0
		}

		Rectangle {
			height: 3
			color: appSettings.fontColor
			Layout.topMargin: 10
			Layout.fillWidth: true
		}

		TPButton {
			id: btnExercises
			text: qsTr("Exercises Database")
			fixedSize: true
			Layout.minimumWidth: drawer.width - 10
			Layout.maximumWidth: drawer.width - 10
			Layout.minimumHeight: 25
			Layout.maximumHeight: 25

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
			fixedSize: true
			Layout.minimumWidth: drawer.width - 10
			Layout.maximumWidth: drawer.width - 10
			Layout.minimumHeight: 25
			Layout.maximumHeight: 25

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
			color: appSettings.fontColor
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
			var button = buttonComponent.createObject(drawerLayout, { text: label, clickId: clickid, associatedItem: page,
				"Layout.minimumWidth": drawer.width - 10, "Layout.maximumWidth": drawer.width - 10, "Layout.minimumHeight": 25,
				"Layout.maximumHeight": 25 });
			button.clicked.connect(function () { itemManager.openMainMenuShortCut();} );
			itemManager.addMainMenuShortCutEntry(button);
		}

		if (buttonComponent.status === Component.Ready)
			finishCreation();
		else
			buttonComponent.statusChanged.connect(finishCreation);
	}
} //Drawer
