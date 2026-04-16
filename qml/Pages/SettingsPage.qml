pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Material

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

	//Maybe TODO
	//Material.theme: Material.Light
	//Material.theme: Material.Dark
	//Material.theme: Material.System

//private
	property bool _need_restart: false
	readonly property list<string> styles: ["Material","Basic", "Fusion","Imagine","macOS","iOS","Universal","Windows","FluentWinUI3"]

	Loader {
		id: restardLoader
		asynchronous: true
		active: settingsPage._need_restart && Qt.platform.os !== "android"

		property TPBalloonTip _dlg

		sourceComponent: TPBalloonTip {
			id: applyTip
			message: qsTr("The App must be restarted in order to reflect the changes")
			imageSource: "settings.png"
			parentPage: settingsPage
			button1Text: qsTr("Restart now")
			button2Text: qsTr("Later");
			onButton1Clicked: AppOsInterface.restartApp();
			onClosed: settingsPage._need_restart = false;
			Component.onCompleted: restardLoader._dlg = this;
		}

		onLoaded: _dlg.showTimed(10000, - Qt.AlignHCenter|Qt.AlignTop);
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

			TPButtonGroup {
				id: stylesGroup
			}

			Repeater {
				id: stylesRepeater
				model: settingsPage.styles.length
				delegateModelAccess: DelegateModel.ReadOnly

				delegate: TPRadioButtonOrCheckBox {
					text: settingsPage.styles[index]
					checked: AppSettings.themeStyle === text;
					buttonGroup: stylesGroup
					Layout.fillWidth: true
					Layout.leftMargin: 10

					required property int index

					onClicked: {
						settingsPage._need_restart = true;
						AppSettings.themeStyle = text;
					}
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
						height: color_rec.height

						onClicked: AppSettings.colorScheme = delegate.index;
						Component.onCompleted: AppSettings.colorChanged.connect(function() {
							checked = AppSettings.colorScheme === delegate.index;
						});
					}

					TPColorRectangle {
						id: color_rec
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
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: settingsPage.width * 0.9

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
