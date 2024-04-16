import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
	id: settingsPage

	property bool bModified: false
	property bool bNeedRestart: false
	property bool bFontSizeChanged: false
	property var appLanguages: [ { text:"English", value: 0 }, { text:"Português", value: 1 }, { text:"Deutsch", value: 2} ]
	property var appLocales: ["en_US", "pt_BR", "de_DE"]
	property int fontPSize : AppSettings.fontSize
	property int optStyleChosen: 0
	property int colorSchemeChosen: 0
	property var colorScheme: []

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

	TPBalloonTip {
		id: applyTip
		message: qsTr("The App must be restarted in order to reflect the changes")
		imageSource: "qrc:/images/"+lightIconFolder+"settings.png"
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

//------------------------------------------------------LANGUAGE------------------------------------------------------
			RowLayout {
				width: parent.width - 60
				spacing: 12
				Layout.alignment: Qt.AlignHCenter
				Layout.fillWidth: true
				Layout.topMargin: 10

				Label {
					text: qsTr("Application Language")
					color: "white"
					font.bold: true
					font.pixelSize: AppSettings.fontSize
				}

				TPComboBox {
					id: cboSetType
					model: appLanguages
					currentIndex: {
						switch (AppSettings.appLocale) {
							case appLocales[0]: return 0;
							case appLocales[1]: return 1;
							case appLocales[2]: return 2;
						}
					}

					onActivated: (index) => {
						bNeedRestart = bModified = true;
					}
				}
			}
//------------------------------------------------------LANGUAGE------------------------------------------------------

			Rectangle {
				height: 3
				color: "white"
				Layout.fillWidth: true
			}

//------------------------------------------------------FONTS------------------------------------------------------
			Label {
				text: qsTr("Fonts Sizes")
				color: "white"
				font.bold: true
				font.pixelSize: AppSettings.fontSize
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
					font.pixelSize: 10
					font.weight: 400
					color: "white"
				}

				Slider {
					id: fontSizeSlider
					snapMode: Slider.SnapAlways
					stepSize: 1
					from: 10
					value: fontPSize
					to: 30
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
					font.pixelSize: 30
					font.weight: 400
					color: "white"
				}
			} //RowLayout

			Label {
				id: lblExample
				text: qsTr("Example text")
				font.pointSize: fontPSize
				color: "white"
				Layout.leftMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}

			Label {
				id: lblTitleFont
				text: qsTr("Font used in titles")
				font.pointSize: fontPSize * 1.5
				color: "white"
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				id: lblListsFont
				text: qsTr("Font used in lists")
				font.pointSize: fontPSize * 0.7
				color: "white"
				Layout.leftMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
			Label {
				id: lblTextFont
				text: qsTr("Font used on text input fields")
				font.pointSize: fontPSize * 0.9
				color: "white"
				elide: Text.ElideRight
				Layout.leftMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
			}
//------------------------------------------------------FONTS------------------------------------------------------

			Rectangle {
				height: 3
				color: "white"
				Layout.fillWidth: true
			}

//------------------------------------------------------THEME------------------------------------------------------
			GroupBox {
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10
				Layout.maximumWidth: settingsPage.width - 20
				spacing: 0
				padding: 0

				label: Label {
					text: qsTr("Application style")
					color: "white"
					anchors.horizontalCenter: parent.horizontalCenter
					anchors.bottomMargin: 10
					font.bold: true
					font.pixelSize: AppSettings.fontSize
				}

				background: Rectangle {
					color: "transparent"
					border.color: "white"
					radius: 6
				}

				ColumnLayout {
					anchors.fill: parent
					spacing: 0

					TPRadioButton {
						id: optBasic
						checked: AppSettings.themeStyle === "Basic";
						text: qsTr("Basic")
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 1;
						}
					}

					TPRadioButton {
						id: optFusion
						checked: AppSettings.themeStyle === "Fusion";
						text: qsTr("Fusion")
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 2;
						}
					}

					TPRadioButton {
						id: optImagine
						checked: AppSettings.themeStyle === "Imagine";
						text: qsTr("Imagine")
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 3;
						}
					}

					TPRadioButton {
						id: optMaterial
						checked: AppSettings.themeStyle === "Material";
						text: qsTr("Material")
						Layout.leftMargin: 10

						onClicked: {
							bNeedRestart = bModified = true;
							if (checked) optStyleChosen = 4;
						}
					}

					TPRadioButton {
						id: optUniversal
						checked: AppSettings.themeStyle === "Universal";
						text: qsTr("Universal")
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
				color: "white"
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

				label: Label {
					text: qsTr("Color Scheme")
					color: "white"
					anchors.horizontalCenter: parent.horizontalCenter
					anchors.bottomMargin: 10
					font.bold: true
					font.pixelSize: AppSettings.fontSize
				}

				background: Rectangle {
					color: "transparent"
					border.color: "white"
					radius: 6
				}

				GridLayout {
					anchors.fill: parent
					columns: 2
					rows: 5

					TPRadioButton {
						id: optBlue
						checked: AppSettings.colorScheme === "Blue";
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
						checked: AppSettings.colorScheme === "Green";
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
						checked: AppSettings.colorScheme === "Red";
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
						checked: AppSettings.colorScheme === "Dark";
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
						midColor: "#b1bab7"
						lightColor: "#d7e2de"
						Layout.column: 1
						Layout.row: 3
						Layout.rightMargin: 10
					}

					TPRadioButton {
						id: optLight
						checked: AppSettings.colorScheme === "Light";
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
		id: mesoCycleToolBar
		width: parent.width
		height: 55

		background: Rectangle {
			color: AppSettings.primaryDarkColor
			opacity: 0.7
		}

		ButtonFlat {
			id: btnApplyChanges
			text: qsTr("Apply")
			enabled: bModified
			width: 80
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter

			onClicked: {
				bModified = false;
				if (bNeedRestart) {
					applyTip.showTimed(4000, 0);
					bNeedRestart = false;
				}

				AppSettings.appLocale = appLocales[cboSetType.currentIndex];

				if (bFontSizeChanged) {
					AppSettings.fontSize = fontPSize;
					AppSettings.fontSizeTitle = fontPSize * 1.5
					AppSettings.fontSizeLists = fontPSize * 0.7
					AppSettings.fontSizeText = fontPSize * 0.9
					bFontSizeChanged = false;
				}

				if (optStyleChosen !== 0) {
					switch (optStyleChosen) {
						case 1:
							AppSettings.themeStyle = "Basic";
						break;
						case 2:
							AppSettings.themeStyle = "Fusion";
						break;
						case 3:
							AppSettings.themeStyle = "Imagine";
						break;
						case 4:
							AppSettings.themeStyle = "Material";
						break;
						case 5:
							AppSettings.themeStyle = "Universal";
						break;
					}
					optStyleChosen = 0;
				}

				if (colorSchemeChosen !== 0) {
					switch (colorSchemeChosen) {
						case 1:
							AppSettings.colorScheme = "Blue";
							colorScheme = [recColor1.darkColor, recColor1.midColor, recColor1.lightColor, "#1976d2", "#6495ed"];
						break;
						case 2:
							AppSettings.colorScheme = "Green";
							colorScheme = [recColor2.darkColor, recColor2.midColor, recColor2.lightColor, "#60d219", "#228b22"];
						break;
						case 3:
							AppSettings.colorScheme = "Red";
							colorScheme = [recColor3.darkColor, recColor3.midColor, recColor3.lightColor, "#d22222", "#f08080"];
						break;
						case 4:
							AppSettings.colorScheme = "Dark";
							colorScheme = [recColor4.darkColor, recColor4.midColor, recColor4.lightColor, "#3f0000", "#696969"];
						break;
						case 5:
							AppSettings.colorScheme = "Light";
							colorScheme = [recColor5.darkColor, recColor5.midColor, recColor5.lightColor, "#b3b3b3", "#b0c4de"];
						break;
					}
					colorSchemeChosen = 0;
					AppSettings.primaryDarkColor = colorScheme[0];
					AppSettings.primaryColor = colorScheme[1];
					AppSettings.primaryLightColor = colorScheme[2];
					AppSettings.paneBackgroundColor = colorScheme[3];
					AppSettings.entrySelectedColor = colorScheme[4];
				}
			}
		}
	}
}
