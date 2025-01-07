import QtQuick
import QtQuick.Controls

import "../"

Rectangle {
	width: 25
	height: 35
	color: appSettings.primaryColor
	enabled: false

	property alias value: list.currentIndex
	property int max

	Rectangle {
		anchors.fill: parent
		gradient: Gradient {
			orientation: Gradient.Vertical
			GradientStop { position: 0.0; color: appSettings.primaryLightColor }
			GradientStop { position: 0.2; color: appSettings.primaryDarkColor }
			GradientStop { position: 0.8; color: appSettings.primaryDarkColor }
			GradientStop { position: 1.0; color: appSettings.primaryLightColor }
		}
	}

	ListView {
		id: list
		anchors.fill: parent
		highlightRangeMode: ListView.StrictlyEnforceRange
		preferredHighlightBegin: height / 4
		preferredHighlightEnd: height / 3
		clip: true
		model: max
		delegate: Text {
			minimumPixelSize: 8
			fontSizeMode: Text.Fit
			horizontalAlignment: Text.AlignHCenter
			leftPadding: 5
			font.bold: true
			color: appSettings.fontColor
			text: String(index).length == 1 ? "0" + index : index
			//anchors.horizontalCenter: parent.horizontalCenter
		}
	}
}
