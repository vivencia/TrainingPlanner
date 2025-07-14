import QtQuick
import QtQuick.Controls

import "../"

Item {
	id: control
	height: lblText.contentHeight
	implicitHeight: height

	property alias text: lblText.text
	property alias textColor: lblText.color
	property bool checked
	property bool multiLine: false
	property bool actionable: true

	signal clicked();
	signal pressAndHold();

	Rectangle {
		id: indicator
		implicitWidth: appSettings.itemDefaultHeight * 0.8
		implicitHeight: implicitWidth
		radius: 4
		color: "transparent"
		border.color: control.enabled ? textColor : appSettings.disabledFontColor

		anchors {
			left: parent.left
			verticalCenter: lblText.verticalCenter
		}

		Rectangle {
			id: recChecked
			width: appSettings.itemDefaultHeight * 0.5
			height: width
			x: (indicator.implicitWidth - width) * 0.5
			y: x
			radius: 2
			border.color: control.enabled ? textColor : appSettings.disabledFontColor
			visible: control.checked
		}
	}

	TPLabel {
		id: lblText
		text: control.text
		wrapMode: multiLine ? Text.WordWrap : Text.NoWrap
		topPadding: 5
		bottomPadding: 5
		leftPadding: 0
		rightPadding: 0

		anchors {
			top: parent.top
			left: indicator.right
			leftMargin: 5
			right: parent.right
		}
	}

	MouseArea {
		enabled: actionable
		anchors.fill: parent

		onClicked: {
			control.checked = !control.checked;
			control.clicked();
		}

		onPressAndHold: control.pressAndHold();
	}
}
