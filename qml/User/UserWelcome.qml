import QtQuick
import QtQuick.Controls

import "../TPWidgets"
import ".."

TPLabel {
	id: lblMain
	text: qsTr("Welcome to the Training Planner app!\nLet's first setup the application by creating your profile.")
	wrapMode: Text.WordWrap
	horizontalAlignment: Text.AlignJustify
	verticalAlignment: Text.AlignVCenter
	font: AppGlobals.extraLargeFont

	readonly property bool bReady: true

	topPadding: 20
	leftPadding: 30
	rightPadding: 30
}
