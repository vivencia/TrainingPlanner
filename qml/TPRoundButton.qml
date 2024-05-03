import QtQuick
import QtQuick.Controls

RoundButton {
	id: control
	focusPolicy: Qt.NoFocus

	property string imageName

	Image {
		source: "qrc:/images/"+darkIconFolder+imageName
		asynchronous: true
		fillMode: Image.PreserveAspectFit
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter
		height: control.height - 5
		width: control.width - 5
		opacity: control.enabled ? 1 : 0.5
	}
}
