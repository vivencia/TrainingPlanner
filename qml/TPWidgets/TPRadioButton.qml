import QtQuick
import QtQuick.Controls

import "../"

RadioButton {
	id: control
	spacing: 5
	padding: 0
	implicitHeight: lblText.preferredHeight + 5
	implicitWidth: lblText.preferredWidth + indicator.width + spacing

	property alias textColor: lblText.enabledFontColor

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
