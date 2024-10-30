import QtQuick
import QtQuick.Controls

import "../"

CheckBox {
	id: control
	spacing: 5
	padding: 0

	property alias textColor: lblText.color

	contentItem: TPLabel {
		id: lblText
		text: control.text
		wrapMode: Text.WordWrap
		leftPadding: control.indicator.width + control.spacing
	}

	indicator: Rectangle {
		implicitWidth: 20
		implicitHeight: 20
		x: 3
		y: lblText.y
		radius: 4
		color: "transparent"
		border.color: control.enabled ? textColor : "darkgray"

		Rectangle {
			width: 10
			height: 10
			x: 5
			y: 5
			radius: 2
			color: control.checked ? control.enabled ? textColor : "darkgray" : "transparent"
		}
	}
}
