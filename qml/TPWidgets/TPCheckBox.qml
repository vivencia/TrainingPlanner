import QtQuick
import QtQuick.Controls

import "../"

Item {
	id: control
	implicitHeight: lblText.height

	property alias text: lblText.text
	property alias textColor: lblText.color
	property bool checked
	property bool multiLine: false
	property bool actionable: true

	signal clicked();
	signal pressAndHold();

	Rectangle {
		id: indicator
		implicitWidth: 20
		implicitHeight: 20
		radius: 4
		color: "transparent"
		border.color: control.enabled ? textColor : appSettings.disabledFontColor

		anchors {
			left: parent.left
			verticalCenter: lblText.verticalCenter
		}

		Rectangle {
			id: recChecked
			width: 10
			height: 10
			x: 5
			y: 5
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

		Component.onCompleted: {
			if (!multiLine)
			{
				adjustTextSize();
				if (_textWidth > control.width)
					wrapMode = Text.WordWrap;
			}
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
