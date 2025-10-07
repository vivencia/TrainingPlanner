import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import "../"
import "../TPWidgets"
import "../User"

TPPage {
	id: settingsPage
	objectName: "settingsPage"
	implicitWidth: appSettings.pageWidth
	implicitHeight: appSettings.pageHeight

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
				Layout.topMargin: 20
				Layout.bottomMargin: 20
			}

//------------------------------------------------------APP BEHAVIOUR------------------------------------------------------

			TPRadioButtonOrCheckBox {
				id: chkAskConfirmation
				text: qsTr("Always ask the user confirmation before starting any - potencially destructive - action")
				multiLine: true
				radio: false
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
				Layout.topMargin: 20
				Layout.bottomMargin: 20
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
					font.pixelSize: 30
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
				Layout.topMargin: 20
				Layout.bottomMargin: 20
			}

//------------------------------------------------------THEME------------------------------------------------------
			TPLabel {
				text: qsTr("Application style")
				Layout.alignment: Qt.AlignCenter
			}

			TPRadioButtonOrCheckBox {
				text: "Material"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}

			TPRadioButtonOrCheckBox {
				text: "Basic"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}

			TPRadioButtonOrCheckBox {
				text: "Fusion"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}

			TPRadioButtonOrCheckBox {
				text: "Imagine"
				checked: appSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					bNeedRestart = true;
					appSettings.themeStyle = text;
				}
			}

			TPRadioButtonOrCheckBox {
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
				Layout.topMargin: 20
				Layout.bottomMargin: 20
			}

//------------------------------------------------------COLORS------------------------------------------------------
			TPLabel {
				text: qsTr("Color Scheme")
				Layout.alignment: Qt.AlignCenter
				Layout.bottomMargin: 20
			}

			Repeater {
				id: colorSchemeRepeater
				model: appSettings.colorSchemes

				Row {
					spacing: 20
					Layout.fillWidth: true
					Layout.topMargin: 10
					Layout.leftMargin: 10
					Layout.rightMargin: 10

					required property int index

					TPRadioButtonOrCheckBox {
						text: appSettings.colorSchemes[index]
						checked: appSettings.colorScheme === index
						multiLine: index === 0
						width: parent.width*0.6

						onClicked: appSettings.colorScheme = index;
						Component.onCompleted: appSettings.colorChanged.connect(function() { checked = appSettings.colorScheme === index; });
					}

					TPColorRectangle {
						midColor: appSettings.colorForScheme(index)
						lightColor: appSettings.lightColorForScheme(index)
						darkColor: appSettings.darkColorForScheme(index)
						clickable: index === 0
						width: parent.width*0.3
					}
				}
			}
//------------------------------------------------------COLORS------------------------------------------------------

			Rectangle {
				height: 3
				color: appSettings.fontColor
				Layout.fillWidth: true
			}

//------------------------------------------------------FONT-COLORS------------------------------------------------------
			TPLabel {
				text: qsTr("Font Color")
				Layout.alignment: Qt.AlignCenter
			}

			Row {
				spacing: 0
				padding: 0
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10

				TPLabel {
					id: lblEnabled
					text: qsTr("Enabled text")
					width: parent.width * 0.4
				}
				TPButton {
					imageSource: "color-choose"
					width: appSettings.itemDefaultHeight
					height: width
					Layout.leftMargin: -5

					onClicked: colorDlg.show(lblEnabled);
				}

				TPLabel {
					id: lblDisabled
					text: qsTr("Disabled text")
					enabled: false
					width: parent.width * 0.4

					Layout.leftMargin: 20
				}
				TPButton {
					imageSource: "color-choose"
					width: appSettings.itemDefaultHeight
					height: width
					Layout.leftMargin: 5

					onClicked: colorDlg.show(lblDisabled);
				}
			}


			TPButton {
				id: btnCustomFontColor
				text: qsTr("Use selected colors")
				autoSize: true
				Layout.alignment: Qt.AlignCenter

				onClicked: {
					appSettings.fontColor = lblEnabled.color;
					appSettings.disabledFontColor = lblDisabled.color;
					enabled = false;
				}
			}

			ColorDialog {
				id: colorDlg

				property TPLabel testLabel

				onAccepted: {
					if (testLabel.color !== selectedColor) {
						testLabel.color = selectedColor;
						btnCustomFontColor.enabled = true;
					}
				}

				function show(label: TPLabel): void {
					testLabel = label;
					selectedColor = testLabel.color;
					open();
				}
			}
//------------------------------------------------------FONT-COLORS------------------------------------------------------
			Item { //Empty item to clear space for the page swipe indicators
				height: 30
			}
		} //ColumnLayout
	} //ScrollView
}
