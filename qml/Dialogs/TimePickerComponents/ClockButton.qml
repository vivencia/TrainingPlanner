import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

Button {
	id: _control
	focusPolicy: Qt.NoFocus
	font.bold: checked
	checkable: true

//public:
	required property int indexInGroup
	required property int nButtonsInGroup
	required property int buttonGroupSize
	readonly property real angle: 360 * (indexInGroup / nButtonsInGroup)

	contentItem: Item {}
	TPLabel {
		text: _control.text
		rotation: -_control.angle
		opacity: _control.checked ? 1.0 : enabled || _control.highlighted ? 1.0 : 0.6
		horizontalAlignment: Text.AlignHCenter
		anchors.centerIn: _control
	} // content Label

	background: Rectangle {
		color: _control.checked ? AppSettings.primaryColor : "transparent"
		radius: width / 2
	}

	transform: [
		Translate {
			y: -_control.buttonGroupSize * 0.5 + _control.height / 2
		},
		Rotation {
			angle: _control.angle
			origin.x: _control.width / 2
			origin.y: _control.height / 2
		}
	]
}
