import QtQuick
import QtQuick.Controls

import "../TPWidgets"
import ".."

TPLabel {
	id: lblMain
	text: qsTr("Welcome to the Training Planner app!\nLet's first setup the application by creating your profile.")
	font: AppGlobals.titleFont

	readonly property bool bReady: true

	topPadding: (height - width)/2
	leftPadding: 30
	rightPadding: 30
}
