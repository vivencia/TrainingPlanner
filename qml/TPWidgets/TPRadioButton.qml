import QtQuick
import QtQuick.Controls

import "../"

RadioButton {
	id: control
	spacing: 5
	padding: 0

	property alias textColor: lblText.color

	contentItem: TPLabel {
		id: lblText
		text: control.text
		wrapMode: Text.WordWrap
		leftPadding: control.indicator.width + control.spacing
		widthAvailable: control.width - leftPadding
	}

	indicator: Rectangle {
		implicitWidth: 20
		implicitHeight: 20
		x: 3
		y: lblText.y
		radius: 10
		color: "transparent"
		border.color: control.enabled ? textColor : "darkgray"

		Rectangle {
			width: 14
			height: 14
			x: 3
			y: 3
			radius: 7
			color: control.checked ? control.enabled ? textColor : "darkgray" : "transparent"
		}
	}
}
