import QtQuick
import QtQuick.Controls

import "../TPWidgets"
import ".."

TPLabel {
	id: lblMain
	text: qsTr("Ready to start using the App. Begin by creating a workout schedule, either within a fixed period of time or an open ended schedule")
	font: AppGlobals.titleFont

	readonly property bool bReady: true

	topPadding: (height - width)/2
	leftPadding: 30
	rightPadding: 30

	function focusOnFirstField() {
		return;
	}
}
