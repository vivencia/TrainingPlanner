import QtQuick

import TpQml
import TpQml.Widgets

TPLabel {
	id: lblMain
	text: qsTr("Welcome to the Training Planner app!<br>Let's first setup the application by creating your profile.")
	horizontalAlignment: Text.AlignJustify
	font: AppGlobals.extraLargeFont
	singleLine: false

	readonly property bool bReady: true

	topPadding: 20
	leftPadding: 30
	rightPadding: 30
}
