import QtQuick
import QtQuick.Controls

import "../"

Rectangle {
	id: control
	width: appSettings.itemDefaultHeight * 1.2
	height: appSettings.itemDefaultHeight * 1.3
	enabled: false

	property alias value: list.currentIndex
	property int max

	gradient: Gradient {
		orientation: Gradient.Vertical
		GradientStop { position: 0.0; color: appSettings.primaryLightColor }
		GradientStop { position: 0.2; color: appSettings.primaryDarkColor }
		GradientStop { position: 0.8; color: appSettings.primaryDarkColor }
		GradientStop { position: 1.0; color: appSettings.primaryLightColor }
	}

	ListView {
		id: list
		anchors.fill: parent
		highlightRangeMode: ListView.StrictlyEnforceRange
		preferredHighlightBegin: height / 4
		preferredHighlightEnd: height / 3
		clip: true
		model: max
		delegate: Label {
			text: String(index).length == 1 ? "0" + index : index
			verticalAlignment: Text.AlignVCenter
			color: enabled ? appSettings.fontColor : appSettings.disabledFontColor
			leftPadding: (control.width - AppGlobals.fontMetricsRegular.boundingRect(text).width) / 2
			font.bold: true
			font.pixelSize: appSettings.fontSize
		}
	}
}
