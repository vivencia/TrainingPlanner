import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import TpQml
import TpQml.Widgets
import TpQml.User

TPPage {
	id: settingsPage
	objectName: "settingsPage"
	imageSource: AppSettings.settingsBackground
	backgroundOpacity: 0.6
	implicitWidth: AppSettings.pageWidth
	implicitHeight: AppSettings.pageHeight

	property bool bNeedRestart: false

	TPBalloonTip {
		id: applyTip
		message: qsTr("The App must be restarted in order to reflect the changes")
		imageSource: "settings.png"
		parentPage: settingsPage

		function init() {
			if (Qt.platform.os !== "android") {
				button1Text = qsTr("Restart now");
				applyTip.button1Clicked.connect(function() { AppOsInterface.restartApp(); });
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

	TPScrollView {
		parentPage: settingsPage
		contentHeight: colMain.implicitHeight
		anchors.fill: parent

		ColumnLayout {
			id: colMain
			anchors.fill: parent
			anchors.margins: 5
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
				Layout.preferredWidth: AppSettings.pageWidth - 20
			}
//------------------------------------------------------LANGUAGE------------------------------------------------------

			Rectangle {
				color: AppSettings.fontColor
				Layout.preferredHeight: 3
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
				checked: AppSettings.alwaysAskConfirmation
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 20

				onClicked: AppSettings.alwaysAskConfirmation = checked;
			}
//------------------------------------------------------APP BEHAVIOUR------------------------------------------------------

			Rectangle {
				color: AppSettings.fontColor
				Layout.preferredHeight: 3
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
				Layout.preferredWidth: parent.width - 60
				Layout.alignment: Qt.AlignHCenter
				Layout.leftMargin: 10
				Layout.rightMargin: 10

				Label {
					id: lblMin
					text: "A"
					font.pixelSize: 8
					font.weight: 400
					color: AppSettings.fontColor
				}

				Slider {
					id: fontSizeSlider
					snapMode: Slider.SnapAlways
					stepSize: 1
					from: 8
					value: AppSettings.fontSize
					to: 40
					Layout.preferredWidth: settingsPage.width - lblMin.width - lblMax.width - 20
					onMoved: AppSettings.fontSize = value;
				}

				Label {
					id: lblMax
					text: "A"
					font.pixelSize: 30
					font.weight: 400
					color: AppSettings.fontColor
				}
			} //RowLayout

			Label {
				text: qsTr("Extra large font")
				font.pixelSize: AppSettings.extraLargeFontSize
				color: AppSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				text: qsTr("Large font")
				font.pixelSize: AppSettings.largeFontSize
				color: AppSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				text: qsTr("Normal font")
				font.pixelSize: AppSettings.fontSize
				color: AppSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				text: qsTr("Small font")
				font.pixelSize: AppSettings.smallFontSize
				color: AppSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
//------------------------------------------------------FONTS------------------------------------------------------

			Rectangle {
				color: AppSettings.fontColor
				Layout.preferredHeight: 3
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
				checked: AppSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					settingsPage.bNeedRestart = true;
					AppSettings.themeStyle = text;
				}
			}

			TPRadioButtonOrCheckBox {
				text: "Basic"
				checked: AppSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					settingsPage.bNeedRestart = true;
					AppSettings.themeStyle = text;
				}
			}

			TPRadioButtonOrCheckBox {
				text: "Fusion"
				checked: AppSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					settingsPage.bNeedRestart = true;
					AppSettings.themeStyle = text;
				}
			}

			TPRadioButtonOrCheckBox {
				text: "Imagine"
				checked: AppSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					settingsPage.bNeedRestart = true;
					AppSettings.themeStyle = text;
				}
			}

			TPRadioButtonOrCheckBox {
				text: "Universal"
				checked: AppSettings.themeStyle === text;
				Layout.fillWidth: true
				Layout.leftMargin: 10

				onClicked: {
					settingsPage.bNeedRestart = true;
					AppSettings.themeStyle = text;
				}
			}
//------------------------------------------------------THEME------------------------------------------------------

			Rectangle {
				color: AppSettings.fontColor
				Layout.preferredHeight: 3
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
				model: AppSettings.colorSchemes

				delegate: Row {
					id: delegate
					spacing: 20
					Layout.fillWidth: true
					Layout.topMargin: 10
					Layout.leftMargin: 10
					Layout.rightMargin: 10

					required property int index

					TPRadioButtonOrCheckBox {
						text: AppSettings.colorSchemes[delegate.index]
						checked: AppSettings.colorScheme === delegate.index
						multiLine: delegate.index === 0
						width: parent.width * 0.6

						onClicked: AppSettings.colorScheme = delegate.index;
						Component.onCompleted: AppSettings.colorChanged.connect(function() {
							checked = AppSettings.colorScheme === delegate.index;
						});
					}

					TPColorRectangle {
						midColor: AppSettings.colorForScheme(delegate.index)
						lightColor: AppSettings.lightColorForScheme(delegate.index)
						darkColor: AppSettings.darkColorForScheme(delegate.index)
						clickable: delegate.index === 0
						width: parent.width * 0.3
					}
				}
			}
//------------------------------------------------------COLORS------------------------------------------------------

			Rectangle {
				color: AppSettings.fontColor
				Layout.preferredHeight: 3
				Layout.fillWidth: true
				Layout.topMargin: 20
				Layout.bottomMargin: 20
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
					width: AppSettings.itemDefaultHeight
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
					width: AppSettings.itemDefaultHeight
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
					AppSettings.fontColor = lblEnabled.color;
					AppSettings.disabledFontColor = lblDisabled.color;
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
				Layout.minimumHeight: 30
			}
		} //ColumnLayout
	} //ScrollView
}
