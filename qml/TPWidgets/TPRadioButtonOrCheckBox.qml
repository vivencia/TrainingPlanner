import QtQuick
import QtQuick.Controls

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"

TPLabel {
	id: control
	height: appSettings.itemDefaultHeight + 10

	property alias image: img.source
	property int imageHeight: appSettings.itemDefaultHeight
	property int imageWidth: imageHeight
	property bool checked: false
	property bool multiLine: false
	property bool actionable: enabled
	property bool radio: true
	property TPButtonGroup buttonGroup: null

	signal clicked();
	signal pressAndHold();

	singleLine: !multiLine
	topPadding: 0
	rightPadding: 0
	bottomPadding: 5
	leftPadding: indicator.width + 10 + (image.length > 0 ? img.width : 0)

	Rectangle {
		id: indicator
		implicitWidth: appSettings.itemSmallHeight
		implicitHeight: implicitWidth
		radius: control.radio ? implicitWidth / 2 : 4
		color: "transparent"
		border.color: control.enabled ? control.color : appSettings.disabledFontColor

		anchors {
			left: control.left
			verticalCenter: control.verticalCenter
			verticalCenterOffset: -5
		}

		Rectangle {
			id: recChecked
			width: indicator.height * 0.5
			height: width
			radius: control.radio ? width * 0.5 : indicator.radius / 2
			x: (indicator.implicitWidth - width) * 0.5
			y: x
			border.color: control.enabled ? control.color : appSettings.disabledFontColor
			visible: control.checked
		}
	}

	TPImage {
		id: img
		height: control.imageHeight
		width: control.imageWidth
		dropShadow: false
		visible: source.length > 0

		anchors {
			left: indicator.right
			leftMargin: 5
			verticalCenter: control.verticalCenter
		}
	}

	MouseArea {
		enabled: control.actionable
		anchors.fill: control

		onClicked: {
			if (!control.radio)
				control.checked = !control.checked;
			else {
				if (control.checked)
					return;
				if (!control.buttonGroup)
					control.checked = true;
				else
					control.buttonGroup.setChecked(control, true)
			}
			control.clicked();
		}

		onPressAndHold: control.pressAndHold();
	}

	Component.onCompleted: {
		if (control.radio && control.buttonGroup)
			control.buttonGroup.addButton(this);
	}

	Component.onDestruction: {
		if (control.radio && control.buttonGroup)
			control.buttonGroup.removeButton(this);
	}
}
