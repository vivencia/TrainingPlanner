import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

TPPage {
	id: settingsPage
	objectName: "settingsPage"

	property bool bNeedRestart: false

	TPBalloonTip {
		id: applyTip
		message: qsTr("The App must be restarted in order to reflect the changes")
		imageSource: "settings.png"
		parentPage: settingsPage

		function init() {
			if (Qt.platform.os !== "android") {
				button1Text = qsTr("Restart now");
				applyTip.button1Clicked.connect(function() { osInterface.restartApp(); });
			}
			showTimed(4000, 0);
		}
	}

	onBNeedRestartChanged: {
		if (bNeedRestart) {
			applyTip.init();
			bNeedRestart = false;
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

			TPLabel {
				text: qsTr("Application Settings")
				font: AppGlobals.extraLargeFont
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
				Layout.topMargin: 20
			}

//------------------------------------------------------LANGUAGE------------------------------------------------------
			UserLanguage {
				width: appSettings.pageWidth - 20
			}
//------------------------------------------------------LANGUAGE------------------------------------------------------

			Rectangle {
				height: 3
				color: appSettings.fontColor
				Layout.fillWidth: true
			}

//------------------------------------------------------APP BEHAVIOUR------------------------------------------------------

			TPCheckBox {
				id: chkAskConfirmation
				text: qsTr("Always ask the user confirmation before starting any - potencially destructive - action")
				checked: appSettings.alwaysAskConfirmation
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 20

				onClicked: appSettings.alwaysAskConfirmation = checked;
			}
//------------------------------------------------------APP BEHAVIOUR------------------------------------------------------

			Rectangle {
				height: 3
				color: appSettings.fontColor
				Layout.fillWidth: true
			}

//------------------------------------------------------FONTS------------------------------------------------------
			TPLabel {
				text: qsTr("Fonts Sizes")
				font: AppGlobals.regularFont
				Layout.alignment: Qt.AlignCenter
				Layout.bottomMargin: 10
			}

			RowLayout {
				id: control
				width: parent.width - 60
				Layout.alignment: Qt.AlignHCenter
				Layout.leftMargin: 10
				Layout.rightMargin: 10

				Label {
					id: lblMin
					text: "A"
					font.pixelSize: 8
					font.weight: 400
					color: appSettings.fontColor
				}

				Slider {
					id: fontSizeSlider
					snapMode: Slider.SnapAlways
					stepSize: 1
					from: 8
					value: appSettings.fontSize
					to: 40
					width: settingsPage.width - lblMin.width - lblMax.width - 20
					onMoved: appSettings.fontSize = value;
				}

				Label {
					id: lblMax
					text: "A"
					font.pixelSize: 40
					font.weight: 400
					color: appSettings.fontColor
				}
			} //RowLayout

			Label {
				text: qsTr("Extra large font")
				font.pixelSize: appSettings.extraLargeFontSize
				color: appSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				text: qsTr("Large font")
				font.pixelSize: appSettings.largeFontSize
				color: appSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				text: qsTr("Normal font")
				font.pixelSize: appSettings.fontSize
				color: appSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				text: qsTr("Small font")
				font.pixelSize: appSettings.smallFontSize
				color: appSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
//------------------------------------------------------FONTS------------------------------------------------------

			Rectangle {
				height: 3
				color: appSettings.fontColor
				Layout.fillWidth: true
			}

//------------------------------------------------------THEME------------------------------------------------------
			TPLabel {
				text: qsTr("Application style")
				Layout.alignment: Qt.AlignCenter
			}

			TPRadioButton {
				text: "Material"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}

			TPRadioButton {
				text: "Basic"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}

			TPRadioButton {
				text: "Fusion"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}

			TPRadioButton {
				text: "Imagine"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}

			TPRadioButton {
				text: "Universal"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}
//------------------------------------------------------THEME------------------------------------------------------

			Rectangle {
				height: 3
				color: appSettings.fontColor
				Layout.fillWidth: true
			}

//------------------------------------------------------COLORS------------------------------------------------------
			TPLabel {
				text: qsTr("Color Scheme")
				Layout.alignment: Qt.AlignCenter
			}

			GridLayout {
				columns: 2
				rows: 6
				uniformCellHeights: true
				uniformCellWidths: true
				Layout.fillWidth: true

				TPRadioButton {
					text: qsTr("Blue")
					checked: appSettings.colorScheme === 0
					Layout.column: 0
					Layout.row: 0
					Layout.leftMargin: 10
					Layout.fillWidth: true

					onClicked: appSettings.colorScheme = 0;
					Component.onCompleted: appSettings.colorChanged.connect(function() { checked = appSettings.colorScheme === 0; });
				}

				TPColorRectangle {
					midColor: appSettings.colorForScheme(0)
					lightColor: appSettings.lightColorForScheme(0)
					darkColor: appSettings.darkColorForScheme(0)
					Layout.column: 1
					Layout.row: 0
					Layout.leftMargin: 20
				}

				TPRadioButton {
					text: qsTr("Green")
					checked: appSettings.colorScheme === 1
					Layout.column: 0
					Layout.row: 1
					Layout.leftMargin: 10
					Layout.fillWidth: true

					onClicked: appSettings.colorScheme = 1;
					Component.onCompleted: appSettings.colorChanged.connect(function() { checked = appSettings.colorScheme === 1; });
				}

				TPColorRectangle {
					midColor: appSettings.colorForScheme(1)
					lightColor: appSettings.lightColorForScheme(1)
					darkColor: appSettings.darkColorForScheme(1)
					Layout.column: 1
					Layout.row: 1
					Layout.leftMargin: 20
				}

				TPRadioButton {
					text: qsTr("Red")
					checked: appSettings.colorScheme === 2
					Layout.column: 0
					Layout.row: 2
					Layout.leftMargin: 10
					Layout.fillWidth: true

					onClicked: appSettings.colorScheme = 2;
					Component.onCompleted: appSettings.colorChanged.connect(function() { checked = appSettings.colorScheme === 2; });
				}

				TPColorRectangle {
					midColor: appSettings.colorForScheme(2)
					lightColor: appSettings.lightColorForScheme(2)
					darkColor: appSettings.darkColorForScheme(2)
					Layout.column: 1
					Layout.row: 2
					Layout.leftMargin: 20
				}

				TPRadioButton {
					text: qsTr("Gray")
					checked: appSettings.colorScheme === 3
					Layout.column: 0
					Layout.row: 3
					Layout.leftMargin: 10
					Layout.fillWidth: true

					onClicked: appSettings.colorScheme = 3;
					Component.onCompleted: appSettings.colorChanged.connect(function() { checked = appSettings.colorScheme === 3; });
				}

				TPColorRectangle {
					midColor: appSettings.colorForScheme(3)
					lightColor: appSettings.lightColorForScheme(3)
					darkColor: appSettings.darkColorForScheme(3)
					Layout.column: 1
					Layout.row: 3
					Layout.leftMargin: 20
				}

				TPRadioButton {
					text: qsTr("Dark")
					checked: appSettings.colorScheme === 4
					Layout.column: 0
					Layout.row: 4
					Layout.leftMargin: 10
					Layout.fillWidth: true

					onClicked: appSettings.colorScheme = 4;
					Component.onCompleted: appSettings.colorChanged.connect(function() { checked = appSettings.colorScheme === 4; });
				}

				TPColorRectangle {
					midColor: appSettings.colorForScheme(4)
					lightColor: appSettings.lightColorForScheme(4)
					darkColor: appSettings.darkColorForScheme(4)
					Layout.column: 1
					Layout.row: 4
					Layout.leftMargin: 20
				}

				TPRadioButton {
					text: qsTr("Light")
					checked: appSettings.colorScheme === 5
					Layout.column: 0
					Layout.row: 5
					Layout.leftMargin: 10
					Layout.fillWidth: true

					onClicked: appSettings.colorScheme = 5;
					Component.onCompleted: appSettings.colorChanged.connect(function() { checked = appSettings.colorScheme === 5; });
				}

				TPColorRectangle {
					midColor: appSettings.colorForScheme(5)
					lightColor: appSettings.lightColorForScheme(5)
					darkColor: appSettings.darkColorForScheme(5)
					Layout.column: 1
					Layout.row: 5
					Layout.leftMargin: 20
				}
			}
//------------------------------------------------------COLORS------------------------------------------------------
		} //ColumnLayout
	} //ScrollView
}
