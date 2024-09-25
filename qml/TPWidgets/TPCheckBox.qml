import QtQuick
import QtQuick.Controls

import "../"

CheckBox {
	id: control
	spacing: 5
	topPadding: 5
	leftPadding: 5
	bottomPadding: 0
	rightPadding: 0
	implicitHeight: Math.ceil(fontMetrics.boundingRect(text).width / 25) + 25
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
