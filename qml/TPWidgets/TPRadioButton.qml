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
	property bool actionable: true

	signal clicked();
	signal pressAndHold();

	Rectangle {
		id: indicator
		implicitWidth: 20
		implicitHeight: 20
		radius: 10
		color: "transparent"
		border.color: control.enabled ? textColor : appSettings.disabledFontColor

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
			border.color: control.enabled ? textColor : appSettings.disabledFontColor
			visible: control.checked
		}
	}

	TPLabel {
		id: lblText
		wrapMode: multiLine ? Text.WordWrap : Text.NoWrap
		//width: Math.max(control.implicitWidth, control.width)
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
			bottom: parent.bottom
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
		enabled: actionable
		anchors.fill: parent
		z: 2

		onClicked: {
			control.checked = !control.checked;
			control.clicked();
		}

		onPressAndHold: control.pressAndHold();
	}
}
