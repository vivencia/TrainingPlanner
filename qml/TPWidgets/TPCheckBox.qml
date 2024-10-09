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
	implicitHeight: lblText.preferredHeight + 5
	implicitWidth: lblText.implicitWidth + indicator.width + spacing

	property string textColor: appSettings.fontColor

	contentItem: TPLabel {
		id: lblText
		text: control.text
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
