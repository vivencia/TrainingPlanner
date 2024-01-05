import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
	id: pageDeveloper

	property int optStyleChosen: 0
	property int optColorChosen: 3
	property bool bClicked: false

	ColumnLayout {
		id: colMain
		anchors.fill: parent
		spacing: 0

		GroupBox {
			title: qsTr("Application style")
			Layout.fillWidth: true
			spacing: 0
			padding: 0

			ColumnLayout {
				anchors.fill: parent
				spacing: 0

				RadioButton {
					id: optBasic
					checked: AppSettings.themeStyle === "Basic";
					text: qsTr("Basic")

					onCheckedChanged: {
						if (checked) optStyleChosen = 1;
					}
				}
				RadioButton {
					id: optFusion
					checked: AppSettings.themeStyle === "Fusion";
					text: qsTr("Fusion")

					onCheckedChanged: {
						if (checked) optStyleChosen = 2;
					}
				}
				RadioButton {
					id: optImagine
					checked: AppSettings.themeStyle === "Imagine";
					text: qsTr("Imagine")

					onCheckedChanged: {
						if (checked) optStyleChosen = 3;
					}
				}
				RadioButton {
					id: optMaterial
					checked: AppSettings.themeStyle === "Material";
					text: qsTr("Material")

					onCheckedChanged: {
						if (checked) optStyleChosen = 4;
					}
				}
				RadioButton {
					id: optUniversal
					checked: AppSettings.themeStyle === "Universal";
					text: qsTr("Universal")

					onCheckedChanged: {
						if (checked) optStyleChosen = 5;
					}
				}
			} //Column Layout
		} //Frame

		GroupBox {
			Layout.fillWidth: true
			title: qsTr("Appearance")
			spacing: 0
			padding: 0

			ColumnLayout {
				spacing: 0
				anchors.fill: parent

				RadioButton {
					id: optSystemTheme
					checked: AppSettings.themeStyleColorIndex === 0
					text: qsTr("Follow System")

					onCheckedChanged: {
						if (checked) optColorChosen = 0;
					}
				}
				RadioButton {
					id: optLightTheme
					checked: AppSettings.themeStyleColorIndex === 1
					text: qsTr("Light Mode")

					onCheckedChanged: {
						if (checked) optColorChosen = 1;
					}
				}
				RadioButton {
					id: optDarkTheme
					checked: AppSettings.themeStyleColorIndex === 2
					text: qsTr("Dark Mode")

					onCheckedChanged: {
						if (checked) optColorChosen = 2;
					}
				}
			}
		} //Group Box

		ToolButton {
			id: btnExecuteAction
			Layout.alignment: Qt.AlignCenter
			text: qsTr("Apply")
			display: AbstractButton.TextUnderIcon
			font.capitalization: Font.MixedCase
			enabled: AppSettings.themeStyleIndex !== optStyleChosen || AppSettings.themeStyleColorIndex !== optColorChosen

			onClicked: {

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
				AppSettings.themeStyleIndex = optStyleChosen;
				AppSettings.themeStyleColorIndex = optColorChosen;
				bClicked = true;
			}
		}

		Label {
			text: qsTr("You must restart the application for the settings to take effect")
			width: pageDeveloper.width - 20
			Layout.maximumWidth: pageDeveloper.width - 20
			Layout.leftMargin: 10
			wrapMode: Text.WordWrap
			visible: bClicked
		}
	} //colMain
}
