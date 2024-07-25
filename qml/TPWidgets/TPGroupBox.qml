import QtQuick
import QtQuick.Controls

import "../"

GroupBox {
	id: control
	padding: 0
	spacing: 0
	property alias text: lblText.text

	label: Label {
		id: lblText
		color: AppSettings.fontColor
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottomMargin: 10
	}

	background: Rectangle {
		id: recBack
		color: "transparent"
		border.color: AppSettings.fontColor
		radius: 6
	}
}
