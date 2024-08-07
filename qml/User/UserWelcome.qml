import QtQuick
import QtQuick.Controls

import ".."

Label {
	id: lblMain
	text: qsTr("Welcome to the Training Planner app!\nLet's first setup the application by creating you profile.")
	wrapMode: Text.WordWrap
	horizontalAlignment: Text.AlignVCenter
	verticalAlignment: Text.AlignHCenter
	font.bold: true
	color: AppSettings.fontColor
	font.pointSize: AppSettings.fontSize

	property int availableWidth
	property int availableHeight
	readonly property bool bReady: true
	property int linearWidth: fontMetrics.boundingRect(text).width + 10

	width: availableWidth - 20
	topPadding: (moduleHeight - Math.ceil(linearWidth/availableWidth) * 30)/2
	leftPadding: 30
	rightPadding: 30

	FontMetrics {
		id: fontMetrics
		font.family: lblMain.font.family
		font.pointSize: lblMain.font.pointSize
		font.weight: lblMain.font.weight
	}
}
