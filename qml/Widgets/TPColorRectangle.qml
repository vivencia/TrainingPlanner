import QtQuick
import QtQuick.Dialogs

import TpQml

Rectangle {
	id: darkRec
	height: width
	border.color: "transparent"
	color: darkColor
	z: 0

	property string darkColor
	property string midColor
	property string lightColor
	property bool clickable: false

	Rectangle {
		id: midRec
		width: darkRec.width*0.6
		height: width
		border.color: "transparent"
		color: darkRec.midColor
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter
		z: 1

		MouseArea {
			enabled: darkRec.clickable
			anchors.fill: parent

			onClicked: colorDlg.show(0, darkRec.midColor);
		}

		Rectangle {
			id: lightRec
			width: midRec.width* 0.5
			height: width
			border.color: "transparent"
			color: darkRec.lightColor
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
			z: 2

			MouseArea {
				enabled: darkRec.clickable
				anchors.fill: parent

				onClicked: colorDlg.show(1, darkRec.lightColor);
			}
		}
	}

	MouseArea {
		enabled: darkRec.clickable
		anchors.fill: parent

		onClicked: colorDlg.show(2, darkRec.darkColor);
	}

	ColorDialog {
		id: colorDlg

		property int _colorIdx

		onAccepted: {
			switch (_colorIdx) {
				case 0:
					darkRec.midColor = selectedColor;
					AppSettings.primaryColor = selectedColor;
				break;
				case 1:
					darkRec.lightColor = selectedColor;
					AppSettings.primaryLightColor = selectedColor;
				break;
				case 2:
					darkRec.darkColor = selectedColor;
					AppSettings.primaryDarkColor = selectedColor;
				break;
			}
		}

		function show(colorIdx: int, initialColor: color): void {
			_colorIdx = colorIdx;
			selectedColor = initialColor;
			open();
		}
	}
}

