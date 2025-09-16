import QtQuick
import QtQuick.Controls

TabButton {
	id: control
	checkable: true
	height: parentTab ? parentTab.height * 0.95 : userSettings.itemDefaultHeight

	property TabBar parentTab: null

	contentItem: Label {
		text: control.text
		elide: Text.ElideRight
		horizontalAlignment: Qt.AlignHCenter
		verticalAlignment: Qt.AlignVCenter
		font.pixelSize: userSettings.smallFontSize
		color: control.enabled ? userSettings.fontColor : userSettings.disabledFontColor
	}

	background: Rectangle {
		border.color: userSettings.fontColor
		opacity: enabled ? 0.9 : 0.4
		color: checked ? userSettings.primaryDarkColor : userSettings.primaryColor
		radius: 5
	}
}
