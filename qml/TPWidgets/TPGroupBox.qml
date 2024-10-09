import QtQuick
import QtQuick.Controls

import "../"

GroupBox {
	id: control
	padding: 0
	spacing: 0
	property alias text: lblText.text

	label: TPLabel {
		id: lblText
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottomMargin: 10
	}

	background: Rectangle {
		id: recBack
		color: "transparent"
		border.color: appSettings.fontColor
		radius: 6
	}
}
