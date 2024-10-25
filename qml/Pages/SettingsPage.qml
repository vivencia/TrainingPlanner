import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPage {
	id: settingsPage
	objectName: "settingsPage"

	property bool bModified: false
	property bool bNeedRestart: false
	property bool bFontSizeChanged: false
	property var appLocales: ["en_US", "pt_BR", "de_DE"]
	property int fontPSize : appSettings.fontSize
	property int optStyleChosen: 0
	property int colorSchemeChosen: 0
	property var colorScheme: []

	ListModel {
		id: appLanguages
		ListElement { text: "English"; value: 0; }
		ListElement { text: "Português"; value: 1; }
		ListElement { text: "Deutsch"; value: 2; }
	}

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
				font: AppGlobals.titleFont
				horizontalAlignment: Text.AlignHCenter
				Layout.fillWidth: true
				Layout.topMargin: 20
			}

//------------------------------------------------------LANGUAGE------------------------------------------------------
			RowLayout {
				spacing: 10
				Layout.alignment: Qt.AlignHCenter
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 20
				Layout.topMargin: 20

				TPLabel {
					text: qsTr("Application Language")
				}

				TPComboBox {
					id: cboAppLanguage
					model: appLanguages

					onActivated: (index) => bModified = true;

					Component.onCompleted: {
						var opt = 0;
						switch (appSettings.appLocale) {
							case appLocales[0]: opt = 0; break;
							case appLocales[1]: opt = 1; break;
							case appLocales[2]: opt = 2; break;
						}
						currentIndex = opt;
					}
				}
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
					font.pointSize: 10
					font.weight: 400
					color: appSettings.fontColor
				}

				Slider {
					id: fontSizeSlider
					snapMode: Slider.SnapAlways
					stepSize: 1
					from: 8
					value: fontPSize
					to: 20
					width: settingsPage.width - lblMin.width - lblMax.width - 20
					onMoved: {
						fontPSize = value;
						bModified = true;
						bFontSizeChanged = true;
					}
				}

				Label {
					id: lblMax
					text: "A"
					font.pointSize: 30
					font.weight: 400
					color: appSettings.fontColor
				}
			} //RowLayout

			Label {
				id: lblTitleFont
				text: qsTr("Font used in titles")
				font.pointSize: fontPSize * 1.2
				color: appSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				id: lblListsFont
				text: qsTr("Font used in lists")
				font.pointSize: fontPSize * 0.8
				color: appSettings.fontColor
				Layout.leftMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				id: lblTextFont
				text: qsTr("Font used on text input fields")
				font.pointSize: fontPSize * 0.9
				color: appSettings.fontColor
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
//------------------------------------------------------FONTS------------------------------------------------------

			Rectangle {
				height: 3
				color: appSettings.fontColor
				Layout.fillWidth: true
			}

//------------------------------------------------------THEME------------------------------------------------------
			TPGroupBox {
				text: qsTr("Application style")
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20

				ColumnLayout {
					anchors.fill: parent
					spacing: 0

					TPRadioButton {
						id: optBasic
						checked: appSettings.themeStyle === "Basic";
						text: "Basic"
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 1;
						}
					}

					TPRadioButton {
						id: optFusion
						checked: appSettings.themeStyle === "Fusion";
						text: "Fusion"
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 2;
						}
					}

					TPRadioButton {
						id: optImagine
						checked: appSettings.themeStyle === "Imagine";
						text: "Imagine"
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 3;
						}
					}

					TPRadioButton {
						id: optMaterial
						checked: appSettings.themeStyle === "Material";
						text: "Material"
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 4;
						}
					}

					TPRadioButton {
						id: optUniversal
						checked: appSettings.themeStyle === "Universal";
						text: "Universal"
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 5;
						}
					}
				} //Column Layout
			} //GroupBox
//------------------------------------------------------THEME------------------------------------------------------

			Rectangle {
				height: 3
				color: appSettings.fontColor
				Layout.fillWidth: true
			}

//------------------------------------------------------COLORS------------------------------------------------------
			GroupBox {
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
				spacing: 0
				padding: 0

				label: TPLabel {
					text: qsTr("Color Scheme")
					anchors.horizontalCenter: parent.horizontalCenter
					anchors.bottomMargin: 10
				}

				background: Rectangle {
					color: "transparent"
					border.color: appSettings.fontColor
					radius: 6
				}

				GridLayout {
					anchors.fill: parent
					columns: 2
					rows: 5

					TPRadioButton {
						id: optBlue
						checked: appSettings.colorScheme === "Blue";
						text: qsTr("Blue")
						Layout.leftMargin: 10
						Layout.column: 0
						Layout.row: 0

						onCheckedChanged: {
							bModified = true;
							if (checked) colorSchemeChosen = 1;
						}
					}

					TPColorRectangle {
						id: recColor1
						darkColor: "#1976D2"
						midColor: "#25b5f3"
						lightColor: "#BBDEFB"
						Layout.column: 1
						Layout.row: 0
						Layout.rightMargin: 10
					}

					TPRadioButton {
						id: optGreen
						checked: appSettings.colorScheme === "Green";
						text: qsTr("Green")
						Layout.leftMargin: 10
						Layout.column: 0
						Layout.row: 1

						onCheckedChanged: {
							bModified = true;
							if (checked) colorSchemeChosen = 2;
						}
					}

					TPColorRectangle {
						id: recColor2
						darkColor: "#12a35a"
						midColor: "#97dd81"
						lightColor: "#d4fdc0"
						Layout.column: 1
						Layout.row: 1
						Layout.rightMargin: 10
					}

					TPRadioButton {
						id: optRed
						checked: appSettings.colorScheme === "Red";
						text: qsTr("Red")
						Layout.leftMargin: 10
						Layout.column: 0
						Layout.row: 2

						onCheckedChanged: {
							bModified = true;
							if (checked) colorSchemeChosen = 3;
						}
					}

					TPColorRectangle {
						id: recColor3
						darkColor: "#fd1c20"
						midColor: "#fd9ab1"
						lightColor: "#ebafc7"
						Layout.column: 1
						Layout.row: 2
						Layout.rightMargin: 10
					}

					TPRadioButton {
						id: optDark
						checked: appSettings.colorScheme === "Dark";
						text: qsTr("Dark")
						Layout.leftMargin: 10
						Layout.column: 0
						Layout.row: 3

						onCheckedChanged: {
							bModified = true;
							if (checked) colorSchemeChosen = 4;
						}
					}

					TPColorRectangle {
						id: recColor4
						darkColor: "#000000"
						midColor: "#9ea6a3"
						lightColor: "#d7e2de"
						Layout.column: 1
						Layout.row: 3
						Layout.rightMargin: 10
					}

					TPRadioButton {
						id: optLight
						checked: appSettings.colorScheme === "Light";
						text: qsTr("Light")
						Layout.leftMargin: 10
						Layout.column: 0
						Layout.row: 4

						onCheckedChanged: {
							bModified = true;
							if (checked) colorSchemeChosen = 5;
						}
					}

					TPColorRectangle {
						id: recColor5
						darkColor: "#c1c1c1"
						midColor: "#cccccc"
						lightColor: "#f3f3f3"
						Layout.column: 1
						Layout.row: 4
						Layout.rightMargin: 10
					}
				}
			}

//------------------------------------------------------COLORS------------------------------------------------------
		} //ColumnLayout
	} //ScrollView

	footer: ToolBar {
		id: splitToolBar
		width: parent.width
		height: footerHeight

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			id: btnApplyChanges
			text: qsTr("Apply")
			enabled: bModified
			width: 80
			flat: false
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter

			onClicked: apply();
		}
	}

	function apply() {
		bModified = false;
		if (bNeedRestart) {
			applyTip.init();
			bNeedRestart = false;
		}

		appTr.switchToLanguage(appLocales[cboAppLanguage.currentIndex]);
		appSettings.alwaysAskConfirmation = chkAskConfirmation.checked;

		if (bFontSizeChanged) {
			appSettings.fontSize = fontPSize;
			appSettings.fontSizeTitle = fontPSize * 1.5
			appSettings.fontSizeLists = fontPSize * 0.7
			appSettings.fontSizeText = fontPSize * 0.9
			bFontSizeChanged = false;
		}

		if (optStyleChosen !== 0) {
			switch (optStyleChosen) {
				case 1:
					appSettings.themeStyle = "Basic";
				break;
				case 2:
					appSettings.themeStyle = "Fusion";
				break;
				case 3:
					appSettings.themeStyle = "Imagine";
				break;
				case 4:
					appSettings.themeStyle = "Material";
				break;
				case 5:
					appSettings.themeStyle = "Universal";
				break;
			}
			optStyleChosen = 0;
		}

		if (colorSchemeChosen !== 0) {
			switch (colorSchemeChosen) {
				case 1:
					appSettings.colorScheme = "Blue";
					appSettings.fontColor = "#ffffff";
					appSettings.iconFolder = "white/"
					colorScheme = [recColor1.darkColor, recColor1.midColor, recColor1.lightColor, "#1976d2", "#6495ed", "dcdcdc"];
				break;
				case 2:
					appSettings.colorScheme = "Green";
					appSettings.fontColor = "#ffffff";
					appSettings.iconFolder = "white/"
					colorScheme = [recColor2.darkColor, recColor2.midColor, recColor2.lightColor, "#60d219", "#228b22", "dcdcdc"];
				break;
				case 3:
					appSettings.colorScheme = "Red";
					appSettings.fontColor = "#ffffff";
					appSettings.iconFolder = "white/"
					colorScheme = [recColor3.darkColor, recColor3.midColor, recColor3.lightColor, "#d22222", "#f08080", "dcdcdc"];
				break;
				case 4:
					appSettings.colorScheme = "Dark";
					appSettings.fontColor = "#ffffff";
					appSettings.iconFolder = "white/"
					colorScheme = [recColor4.darkColor, recColor4.midColor, recColor4.lightColor, "#757575", "#696969", "dcdcdc"];
				break;
				case 5:
					appSettings.colorScheme = "Light";
					appSettings.fontColor = "#000000";
					appSettings.iconFolder = "black/"
					colorScheme = [recColor5.darkColor, recColor5.midColor, recColor5.lightColor, "#b3b3b3", "#b0c4de", "ffffff"];
				break;
			}
			colorSchemeChosen = 0;
			appSettings.primaryDarkColor = colorScheme[0];
			appSettings.primaryColor = colorScheme[1];
			appSettings.primaryLightColor = colorScheme[2];
			appSettings.paneBackgroundColor = colorScheme[3];
			appSettings.entrySelectedColor = colorScheme[4];
			appSettings.disabledFontColor = colorScheme[5];
			appSettings.sync();
		}
	}
}
