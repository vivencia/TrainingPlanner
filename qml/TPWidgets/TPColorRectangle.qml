import QtQuick
import QtQuick.Dialogs

Rectangle {
	property string darkColor
	property string midColor
	property string lightColor
	property bool clickable: false

	id: darkRec
	width: appSettings.pageWidth/3
	height: width
	border.color: "transparent"
	color: darkColor

	Rectangle {
		id: midRec
		width: darkRec.width*0.6
		height: width
		border.color: "transparent"
		color: midColor
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter

		MouseArea {
			enabled: clickable
			anchors.fill: parent
			z: 1

			onClicked: {
				colorDlg.colorIdx = 1;
				colorDlg.open();
			}
		}

		Rectangle {
			id: lightRec
			width: midRec.width* 0.5
			height: width
			border.color: "transparent"
			color: lightColor
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter

			MouseArea {
				enabled: clickable
				anchors.fill: parent
				z: 2

				onClicked: {
					colorDlg.colorIdx = 2;
					colorDlg.open();
				}
			}
		}
	}

	MouseArea {
		enabled: clickable
		anchors.fill: parent
		z: 0

		onClicked: {
			colorDlg.colorIdx = 0;
			colorDlg.open();
		}
	}

	ColorDialog {
		id: colorDlg

		property int colorIdx

		onAccepted: {
			switch (colorIdx) {
				case 0:
					darkColor = selectedColor;
					appSettings.setDarkColorForScheme(selectedColor);
				break;
				case 1:
					midColor = selectedColor;
					appSettings.setColorForScheme(selectedColor);
				break;
				case 2:
					lightColor = selectedColor;
					appSettings.setLightColorForScheme(selectedColor);
				break;
			}
		}
	}
}

