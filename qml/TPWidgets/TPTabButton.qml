import QtQuick
import QtQuick.Controls

TabButton {
	id: control
	checkable: true
	height: parentTab ? parentTab.height * 0.95 : appSettings.itemDefaultHeight

	property TabBar parentTab: null

	contentItem: Label {
		text: control.text
		elide: Text.ElideRight
		horizontalAlignment: Qt.AlignHCenter
		verticalAlignment: Qt.AlignVCenter
		font.pixelSize: appSettings.smallFontSize
		color: control.enabled ? appSettings.fontColor : appSettings.disabledFontColor
	}

	background: Rectangle {
		border.color: appSettings.fontColor
		opacity: enabled ? 0.9 : 0.4
		color: checked ? appSettings.primaryDarkColor : appSettings.primaryColor
		radius: 5
	}
}
