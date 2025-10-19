import QtQuick
import QtQuick.Controls

import "../"

TPBackRec {
	id: control
	useGradient: true
	width: appSettings.itemLargeHeight
	height: width * 1.1
	enabled: false

	property alias value: list.currentIndex
	property int max

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
			color: enabled ? appSettings.fontColor : appSettings.disabledFontColor
			leftPadding: (control.width - AppGlobals.fontMetricsRegular.boundingRect(text).width) / 2
			font.bold: true
			font.pixelSize: appSettings.fontSize
		}
	}
}
