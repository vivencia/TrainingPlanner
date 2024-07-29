import QtQuick
import QtQuick.Controls

import "../"

CheckBox {
	id: control
	spacing: 5
	padding: 0
	implicitHeight: Math.max(lblText.implicitHeight, 25)
	width: lblText.width
	implicitWidth: width

	property string textColor: AppSettings.fontColor

	FontMetrics {
		id: fontMetrics
		font.family: lblText.font.family
		font.pointSize: lblText.font.pointSize
		font.weight: lblText.font.weight
	}

	contentItem: Text {
		id: lblText
		text: control.text
		color: control.enabled ? textColor : "gray"
		wrapMode: Text.WordWrap
		font.pointSize: AppSettings.fontSizeText
		font.weight: Font.ExtraBold
		width: fontMetrics.boundingRect(text).width
		leftPadding: control.indicator.width + control.spacing
	}

	indicator: Rectangle {
		implicitWidth: 20
		implicitHeight: 20
		x: 3
		y: lblText.y
		radius: 4
		color: "transparent"
		border.color: control.enabled ? textColor : "gray"

		Rectangle {
			width: 10
			height: 10
			x: 5
			y: 5
			radius: 2
			color: control.checked ? control.enabled ? textColor : "gray" : "transparent"
		}
	}
}
