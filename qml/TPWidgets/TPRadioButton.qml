import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

RadioButton {
	id: control
	spacing: 5
	padding: 0
	Layout.fillWidth: true

	property string textColor: AppSettings.fontColor

	contentItem: Label {
		id: lblText
		text: control.text
		color: control.enabled ? textColor : "gray"
		wrapMode: Text.WordWrap
		font.pointSize: AppSettings.fontSizeText
		font.weight: Font.ExtraBold
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
			color: control.checked ? control.enabled ? AppSettings.paneBackgroundColor : "gray" : "transparent"
		}
	}
}
