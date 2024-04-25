import QtQuick
import QtQuick.Controls

CheckBox {
	id: control
	padding: 0
	spacing: 5

	property string textColor: AppSettings.fontColor

	indicator: Rectangle {
		implicitWidth: 20
		implicitHeight: 20
		x: control.leftPadding
		y: (control.height-height)/2
		radius: 4
		border.color: AppSettings.fontColor
		color: AppSettings.fontColor

		Rectangle {
			width: 10
			height: 10
			x: 5
			y: 5
			radius: 2
			color: !control.down ? AppSettings.primaryDarkColor : AppSettings.primaryLightColor
			visible: control.checked
		}
	}

	contentItem: Text {
		text: control.text
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		wrapMode: Text.WordWrap
		opacity: enabled ? 1.0 : 0.3
		verticalAlignment: Text.AlignVCenter
		leftPadding: control.indicator.width + control.spacing
		color: textColor
	}
}
