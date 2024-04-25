import QtQuick
import QtQuick.Controls

Rectangle {
	width: 25
	height: 45
	color: AppSettings.primaryColor

	property alias value: list.currentIndex
	property int max

	Rectangle {
		anchors.fill: parent
		gradient: Gradient {
			orientation: Gradient.Vertical
			GradientStop { position: 0.0; color: AppSettings.primaryLightColor }
			GradientStop { position: 0.2; color: AppSettings.primaryDarkColor }
			GradientStop { position: 0.8; color: AppSettings.primaryDarkColor }
			GradientStop { position: 1.0; color: AppSettings.primaryLightColor }
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
			minimumPointSize: 8
			fontSizeMode: Text.Fit
			font.bold: true
			color: AppSettings.fontColor
			text: String(index).length == 1 ? "0" + index : index
		}
	}
}
