import QtQuick
import QtQuick.Controls

import "../"

Rectangle {
	id: control
	width: userSettings.itemLargeHeight
	height: width * 1.1
	enabled: false

	property alias value: list.currentIndex
	property int max

	gradient: Gradient {
		orientation: Gradient.Vertical
		GradientStop { position: 0.0; color: userSettings.primaryLightColor }
		GradientStop { position: 0.2; color: userSettings.primaryDarkColor }
		GradientStop { position: 0.8; color: userSettings.primaryDarkColor }
		GradientStop { position: 1.0; color: userSettings.primaryLightColor }
	}

	ListView {
		id: list
		anchors.fill: parent
		highlightRangeMode: ListView.StrictlyEnforceRange
		preferredHighlightBegin: height / 4
		preferredHighlightEnd: height / 3
		clip: true
		reuseItems: true
		model: max
		delegate: Label {
			text: String(index).length == 1 ? "0" + index : index
			verticalAlignment: Text.AlignVCenter
			color: enabled ? userSettings.fontColor : userSettings.disabledFontColor
			leftPadding: (control.width - AppGlobals.fontMetricsRegular.boundingRect(text).width) / 2
			font.bold: true
			font.pixelSize: userSettings.fontSize
		}
	}
}
