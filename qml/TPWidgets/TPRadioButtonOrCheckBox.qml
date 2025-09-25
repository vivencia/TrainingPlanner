import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"

TPLabel {
	id: control
	height: appSettings.itemDefaultHeight + 10

	property alias image: img.source
	property int imageHeight: appSettings.itemDefaultHeight
	property int imageWidth: image.length > 0 ? imageHeight : 0
	property bool checked: false
	property bool multiLine: false
	property bool actionable: true
	property bool radio: true
	property TPButtonGroup buttonGroup: null

	signal clicked();
	signal pressAndHold();

	wrapMode: multiLine ? Text.WordWrap : Text.NoWrap
	topPadding: 0
	rightPadding: 0
	bottomPadding: 5
	leftPadding: indicator.width + imageWidth + 5

	Rectangle {
		id: indicator
		implicitWidth: appSettings.itemSmallHeight
		implicitHeight: implicitWidth
		radius: radio ? implicitWidth / 2 : 4
		color: "transparent"
		border.color: control.enabled ? control.color : appSettings.disabledFontColor

		anchors {
			left: control.left
			verticalCenter: control.verticalCenter
			verticalCenterOffset: -5
		}

		Rectangle {
			id: recChecked
			width: appSettings.itemDefaultHeight * 0.5
			height: width
			radius: radio ? width * 0.5 : indicator.radius / 2
			x: (indicator.implicitWidth - width) * 0.5
			y: x
			border.color: control.enabled ? control.color : appSettings.disabledFontColor
			visible: control.checked
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
			verticalCenter: control.verticalCenter
			verticalCenterOffset: -5
		}
	}

	MouseArea {
		enabled: actionable
		anchors.fill: control

		onClicked: {
			if (!radio)
				control.checked = !control.checked;
			else {
				if (control.checked)
					return;
				if (!buttonGroup)
					control.checked = true;
				else
					buttonGroup.setChecked(control, true)
			}
			control.clicked();
		}

		onPressAndHold: control.pressAndHold();
	}

	Component.onCompleted: {
		if (radio && buttonGroup)
			buttonGroup.addButton(this);
	}

	Component.onDestruction: {
		if (radio && buttonGroup)
			buttonGroup.removeButton(this);
	}
}
