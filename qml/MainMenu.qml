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

	background: Rectangle {
		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: "#dcfbff"; }
			GradientStop { position: 0.25; color: primaryLightColor; }
			GradientStop { position: 0.50; color: primaryColor; }
			GradientStop { position: 0.75; color: primaryDarkColor; }
		}
		opacity: 0.8
	}

	onClosed: {
		if (!bMenuClicked)
			mainMenuClosed();
	}

	onOpened: {
		mainMenuOpened();
	}

	ColumnLayout {
		id: drawerLayout
		spacing: 5

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 10
		}

		Rectangle {
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignCenter
			height: 200
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
				text: "TrainingPlanner by VivenciaSoftware - v20240116"
				wrapMode: Text.WordWrap
				font.bold: true
				font.pixelSize: AppSettings.fontSizeLists
				color: "white"
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
			color: "white"
		}

		TransparentButton {
			id: btnSettingsExDB
			Layout.fillWidth: true
			text: qsTr("Exercises Database")
			onButtonClicked: { stackView.push("ExercisesDatabase.qml"); menuClicked(); }
		}

		TransparentButton {
			id: btnSettingsTheme
			text: qsTr("Theme")
			Layout.fillWidth: true
			onClicked: { stackView.push("ThemeSettingsPage.qml"); menuClicked(); }
		}

		TransparentButton {
			id: btnSettingsLanguage
			text: qsTr("Language")
			Layout.fillWidth: true
			onClicked: { stackView.push("LanguageSettingsPage.qml", {} ); menuClicked(); }
		}

		TransparentButton {
			id: btnSettingsFont
			text: qsTr("Fonts")
			Layout.fillWidth: true
			onClicked: { stackView.push("FontSizePage.qml"); menuClicked(); }
		}

		TransparentButton {
			id: btnSettingsDev
			text: qsTr("Developer Options")
			Layout.fillWidth: true
			onClicked: { stackView.push("DevSettingsPage.qml"); menuClicked(); }
		}

		Item { // spacer item
			Layout.fillWidth: true
			Layout.fillHeight: true
		}
	} //ColumnLayout

	Component.onCompleted: {
		mainwindow.backButtonPressed.connect(maybeRestore);
	}

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
} //Drawer
