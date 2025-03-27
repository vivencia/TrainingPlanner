import QtQuick
import QtQuick.Dialogs

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
		color: midColor
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter
		z: 1

		MouseArea {
			enabled: clickable
			anchors.fill: parent

			onClicked: colorDlg.show(0, midColor);
		}

		Rectangle {
			id: lightRec
			width: midRec.width* 0.5
			height: width
			border.color: "transparent"
			color: lightColor
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
			z: 2

			MouseArea {
				enabled: clickable
				anchors.fill: parent

				onClicked: colorDlg.show(1, lightColor);
			}
		}
	}

	MouseArea {
		enabled: clickable
		anchors.fill: parent

		onClicked: colorDlg.show(2, darkColor);
	}

	ColorDialog {
		id: colorDlg

		property int _colorIdx

		onAccepted: {
			switch (_colorIdx) {
				case 0:
					midColor = selectedColor;
					appSettings.primaryColor = selectedColor;
				break;
				case 1:
					lightColor = selectedColor;
					appSettings.primaryLightColor = selectedColor;
				break;
				case 2:
					darkColor = selectedColor;
					appSettings.primaryDarkColor = selectedColor;
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

