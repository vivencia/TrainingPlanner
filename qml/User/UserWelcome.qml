import QtQuick
import QtQuick.Controls

import "../TPWidgets"
import ".."

TPLabel {
	id: lblMain
	text: qsTr("Welcome to the Training Planner app!\nLet's first setup the application by creating your profile.")
	horizontalAlignment: Text.AlignJustify
	font: AppGlobals.extraLargeFont
	wrapMode: Text.WordWrap

	readonly property bool bReady: true

	topPadding: 20
	leftPadding: 30
	rightPadding: 30
}
