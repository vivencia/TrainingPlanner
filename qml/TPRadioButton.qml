import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RadioButton {
	id: control
	spacing: 0
	Layout.fillWidth: true

	property string textColor: AppSettings.fontColor

	contentItem: Label {
		text: control.text
		color: textColor
		wrapMode: Text.WordWrap
		font.pointSize: AppSettings.fontSizeText
		font.weight: Font.ExtraBold
		leftPadding: control.indicator.width + control.spacing
	}

	indicator: Rectangle {
		implicitWidth: 20
		implicitHeight: 20
		x: 3
		y: (parent.height-height)/2
		radius: 10
		color: "transparent"
		border.color: textColor

		Rectangle {
			width: 14
			height: 14
			x: 3
			y: 3
			radius: 7
			color: control.checked ? AppSettings.paneBackgroundColor : "transparent"
		}
	}
}
