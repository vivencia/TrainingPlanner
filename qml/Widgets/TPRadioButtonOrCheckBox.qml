import QtQuick

import TpQml

TPLabel {
	id: _control
	height: AppSettings.itemDefaultHeight + 10

//public:
	property alias image: img.source
	property int imageHeight: AppSettings.itemDefaultHeight
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
		implicitWidth: AppSettings.itemSmallHeight
		implicitHeight: implicitWidth
		radius: _control.radio ? implicitWidth / 2 : 4
		color: "transparent"
		border.color: _control.enabled ? _control.color : AppSettings.disabledFontColor

		anchors {
			left: _control.left
			verticalCenter: _control.verticalCenter
			verticalCenterOffset: -5
		}

		Rectangle {
			id: recChecked
			width: indicator.height * 0.5
			height: width
			radius: _control.radio ? width * 0.5 : indicator.radius / 2
			x: (indicator.implicitWidth - width) * 0.5
			y: x
			border.color: _control.enabled ? _control.color : AppSettings.disabledFontColor
			visible: _control.checked
		}
	}

	TPImage {
		id: img
		height: _control.imageHeight
		width: _control.imageWidth
		dropShadow: false
		visible: source.length > 0

		anchors {
			left: indicator.right
			leftMargin: 5
			verticalCenter: _control.verticalCenter
		}
	}

	MouseArea {
		enabled: _control.actionable
		anchors.fill: _control

		onClicked: {
			if (!_control.radio)
				_control.checked = !_control.checked;
			else {
				if (_control.checked)
					return;
				if (!_control.buttonGroup)
					_control.checked = true;
				else
					_control.buttonGroup.setChecked(_control, true)
			}
			_control.clicked();
		}

		onPressAndHold: _control.pressAndHold();
	}

	Component.onCompleted: {
		if (_control.radio && _control.buttonGroup)
			_control.buttonGroup.addButton(this);
	}

	Component.onDestruction: {
		if (_control.radio && _control.buttonGroup)
			_control.buttonGroup.removeButton(this);
	}
}
