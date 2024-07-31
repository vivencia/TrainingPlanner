import QtQuick
import QtQuick.Controls

import "../"

RoundButton {
	id: control
	focusPolicy: Qt.NoFocus
	implicitWidth: width
	implicitHeight: height
	padding: 0
	property string imageName

	Image {
		source: "qrc:/images/"+darkIconFolder+imageName
		asynchronous: true
		fillMode: Image.PreserveAspectFit
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter
		height: control.height - 5
		width: height
		opacity: control.enabled ? 1 : 0.5
	}

	background: Rectangle {
		radius: control.radius
		opacity: enabled ? 1 : 0.3
		visible: !control.flat || control.down || control.checked || control.highlighted
		color: Color.blend(control.checked || control.highlighted ? control.palette.dark : control.palette.button,
																	control.palette.mid, control.down ? 0.5 : 0.0)
		border.color: !control.checked ? "transparent" : AppSettings.fontColor
		border.width: !control.checked ? 0 : 2
	}
}
