import QtQuick
import QtQuick.Controls

import "../"

RadioButton {
	id: control
	spacing: 5
	padding: 0
	implicitHeight: Math.max(lblText.implicitHeight, 25)
	width: fontMetrics.boundingRect(text).width + 25
	implicitWidth: width

	property string textColor: AppSettings.fontColor

	FontMetrics {
		id: fontMetrics
		font.family: lblText.font.family
		font.pointSize: lblText.font.pointSize
		font.weight: lblText.font.weight
	}

	contentItem: Label {
		id: lblText
		text: control.text
		color: control.enabled ? textColor : "gray"
		wrapMode: Text.WordWrap
		font.weight: Font.Bold
		fontSizeMode: Text.Fit
		font.pointSize: AppSettings.fontSizeText
		minimumPointSize: 8
		maximumLineCount: Math.ceil(fontMetrics.boundingRect(text).width / control.width)
		leftPadding: control.indicator.width + control.spacing
		topPadding: fontMetrics.boundingRect(text).width > control.width ? -4 : 2
	}

	indicator: Rectangle {
		implicitWidth: 20
		implicitHeight: 20
		x: 3
		y: lblText.y
		radius: 10
		color: "transparent"
		border.color: control.enabled ? textColor : "gray"

		Rectangle {
			width: 14
			height: 14
			x: 3
			y: 3
			radius: 7
			color: control.checked ? control.enabled ? textColor : "gray" : "transparent"
		}
	}
}
