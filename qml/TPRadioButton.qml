import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RadioButton {
	id: control
	padding: 0
	Layout.fillWidth: true

	property string textColor: "white"

	contentItem: Label {
		text: control.text
		color: textColor
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
		y: (parent.height-height)/2
		radius: 10
		color: "white"

		Rectangle {
			width: 14
			height: 14
			x: 3
			y: 3
			radius: 7
			color: !control.down ? AppSettings.primaryDarkColor : AppSettings.primaryColor
			visible: control.checked
		}
	}
}
