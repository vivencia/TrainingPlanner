import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"

Item {
	id: control
	implicitHeight: lblText.height

	property alias text: lblText.text
	property alias textColor: lblText.color
	property alias image: img.source

	property int imageHeight: 20
	property int imageWidth: 20
	property bool checked
	property bool multiLine: false

	signal clicked();
	signal pressAndHold();

	Rectangle {
		id: indicator
		implicitWidth: 20
		implicitHeight: 20
		radius: 10
		color: "transparent"
		border.color: control.enabled ? textColor : "darkgray"

		anchors {
			left: parent.left
			verticalCenter: lblText.verticalCenter
		}

		Rectangle {
			id: recChecked
			width: 14
			height: 14
			x: 3
			y: 3
			radius: 7
			color: control.enabled ? textColor : "darkgray"
			visible: control.checked
		}
	}

	TPLabel {
		id: lblText
		wrapMode: multiLine ? Text.WordWrap : Text.NoWrap
		topPadding: 5
		bottomPadding: 5
		leftPadding: 0
		rightPadding: 0
		visible: text.length > 0

		anchors {
			top: parent.top
			left: img.visible ? img.right : indicator.right
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

	TPImage {
		id: img
		height: imageHeight
		width: imageWidth
		dropShadow: false
		visible: source.length > 0

		anchors {
			left: indicator.right
			leftMargin: 5
			verticalCenter: lblText.verticalCenter
		}
	}

	MouseArea {
		anchors.fill: parent
		z: 2

		onClicked: {
			control.checked = !control.checked;
			control.clicked();
		}

		onPressAndHold: control.pressAndHold();
	}
}
