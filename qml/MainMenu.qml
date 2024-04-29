import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Drawer {
	id: drawer
	height: mainwindow.height
	implicitWidth: mainwindow.width * 0.6
	spacing: 0
	padding: 0
	edge: Qt.LeftEdge

	property bool bMenuClicked: false
	property var stackWindows: []

	background: Rectangle {
		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
			GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
			GradientStop { position: 0.50; color: AppSettings.primaryColor; }
			GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
		}
		opacity: 0.8
	}

	onClosed: {
		if (!bMenuClicked)
			mainMenuClosed();
	}

	onOpened: mainMenuOpened();

	TransparentButton {
		id: btnExit
		text: qsTr("Exit")
		imageSource: "qrc:/images/"+AppSettings.iconFolder+"application-exit.png"
		leftAlign: false

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
			height: 240
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
				text: "TrainingPlanner by VivenciaSoftware - v20240405"
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
			width: parent.width
			color: AppSettings.fontColor
		}

		TransparentButton {
			id: btnSettingsExDB
			Layout.fillWidth: true
			text: qsTr("Exercises Database")
			onButtonClicked: {
				function pushExercisesPageOntoStack(object) {
					appDB.getPage.disconnect(pushExercisesPageOntoStack);
					stackView.push(object, stackView.DontLoad);
					menuClicked();
				}

				appDB.getPage.connect(pushExercisesPageOntoStack);
				appDB.openExercisesListPage();
			}
		}

		TransparentButton {
			id: btnSettingsTheme
			text: qsTr("Settings")
			Layout.fillWidth: true
			onClicked: { stackView.push("SettingsPage.qml"); menuClicked(); }
		}

		TransparentButton {
			id: btnBackup
			text: qsTr("Backup/Restore")
			Layout.fillWidth: true
			onClicked: { stackView.push("BackupPage.qml"); menuClicked(); }
		}

		Rectangle {
			height: 3
			width: parent.width
			color: AppSettings.fontColor
		}

		Item { // spacer item
			Layout.fillWidth: true
			Layout.fillHeight: true
		}
	} //ColumnLayout

	Component.onCompleted: mainwindow.backButtonPressed.connect(maybeRestore);

	function menuClicked() {
		bMenuClicked = true;
		close ();
	}

	function maybeRestore() {
		if (!drawer.visible && bMenuClicked) {
			drawer.open();
			bMenuClicked = false;
		}
	}

	function createShortCut(label: string, object: Item, clickid: int) {
		var component = Qt.createComponent("TransparentButton.qml", Qt.Asynchronous);

		function finishCreation() {
			var button = component.createObject(drawerLayout, { "text": label, "Layout.fillWidth": true, "clickId": clickid });
			button.buttonClicked.connect(itemManager.openMainMenuShortCut);
			itemManager.addMainMenuShortCutEntry(button);
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}
} //Drawer
