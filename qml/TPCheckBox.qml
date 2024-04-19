import QtQuick
import QtQuick.Controls

CheckBox {
	id: control
	padding: 0
	spacing: 0

	property string textColor: "white"

	indicator: Rectangle {
		implicitWidth: 26
		implicitHeight: 26
		x: control.leftPadding
		y: control.height / 2 - height / 2
		radius: 5
		border.color: control.down ? AppSettings.primaryDarkColor : AppSettings.primaryLightColor
		opacity: 0.5

		Rectangle {
			width: 14
			height: 14
			x: 6
			y: 6
			radius: 2
			color: control.down ? AppSettings.primaryDarkColor : AppSettings.primaryLightColor
			visible: control.checked
			opacity: 0.5
		}
	}

	contentItem: Text {
		text: control.text
		font.pixelSize: AppSettings.fontSizeText
		font.bold: true
		wrapMode: Text.WordWrap
		opacity: enabled ? 1.0 : 0.3
		verticalAlignment: Text.AlignVCenter
		leftPadding: control.indicator.width + control.spacing
		color: textColor
	}
}
