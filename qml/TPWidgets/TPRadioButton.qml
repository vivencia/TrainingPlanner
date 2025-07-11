import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"

Item {
	id: control
	height: lblText.height
	implicitHeight: height

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
		implicitWidth: appSettings.itemDefaultHeight * 0.8
		implicitHeight: implicitWidth
		radius: implicitWidth/2
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
			radius: width * 0.5
			x: (indicator.implicitWidth - width) * 0.5
			y: x
			border.color: control.enabled ? textColor : appSettings.disabledFontColor
			visible: control.checked
		}
	}

	TPLabel {
		id: lblText
		wrapMode: multiLine ? Text.WordWrap : Text.NoWrap
		topPadding: 10
		bottomPadding: 10
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

		onClicked: {
			if (!control.checked) {
				control.checked = true;
				control.clicked();
			}
		}

		onPressAndHold: control.pressAndHold();
	}
}
