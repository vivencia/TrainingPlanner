import QtQuick
import QtQuick.Controls

import "../TPWidgets"
import ".."

TPLabel {
	id: lblMain
	text: qsTr("Ready to start using the App. Begin by creating a workout schedule, either within a fixed period of time or an open ended schedule")
	wrapMode: Text.WordWrap
	horizontalAlignment: Text.AlignJustify
	verticalAlignment: Text.AlignVCenter
	font: AppGlobals.extraLargeFont

	readonly property bool bReady: true

	topPadding: 30
	leftPadding: 30
	rightPadding: 30
}
