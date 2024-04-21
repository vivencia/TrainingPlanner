import QtQuick
import QtQuick.Controls

GroupBox {
	id: control
	padding: 0
	spacing: 0
	property alias text: lblText.text
	property bool highlight: false

	label: Label {
		id: lblText
		color: "white"
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottomMargin: 10
	}

	background: Rectangle {
		id: recBack
		color: "transparent"
		border.color: "white"
		radius: 6
	}

	onHighlightChanged: {
		if (highlight) {
			recBack.border.width = 2;
			anim.start();
		}
		else {
			recBack.border.width = 1;
			anim.stop();
		}
	}

	SequentialAnimation {
		id: anim
		loops: Animation.Infinite

		ColorAnimation {
			target: recBack
			property: "border.color"
			from: "white"
			to: "gold"
			duration: 300
			easing.type: Easing.InOutCubic
		}
		ColorAnimation {
			target: recBack
			property: "border.color"
			from: "gold"
			to: "white"
			duration: 300
			easing.type: Easing.InOutCubic
		}
	}
}
