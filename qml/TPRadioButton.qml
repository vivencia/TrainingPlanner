import QtQuick
import QtQuick.Controls

RadioButton {
	id: control
	padding: 0

	contentItem: Label {
		text: control.text
		color: "white"
		wrapMode: Text.WordWrap
		font.pixelSize: AppSettings.fontSizeText
		font.weight: Font.ExtraBold
		anchors.left: parent.left
		anchors.leftMargin: 25
		anchors.right: parent.right
		anchors.rightMargin: 5
	}

	indicator: Rectangle {
		implicitWidth: 20
		implicitHeight: 20
		x: 3
		y: parent.height / 2 - height / 2
		radius: 10
		border.color: control.down ? primaryDarkColor : primaryLightColor

		Rectangle {
			width: 14
			height: 14
			x: 3
			y: 3
			radius: 7
			color: control.down ? primaryDarkColor : primaryLightColor
			visible: control.checked
		}
	}
}
