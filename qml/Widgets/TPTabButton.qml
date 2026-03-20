import QtQuick
import QtQuick.Controls

import TpQml

TabButton {
	id: _control
	checkable: true
	height: parentTab ? parentTab.height * 0.95 : AppSettings.itemDefaultHeight

	property TabBar parentTab: null

	contentItem: Label {
		text: _control.text
		elide: Text.ElideRight
		horizontalAlignment: Qt.AlignHCenter
		verticalAlignment: Qt.AlignVCenter
		font.pixelSize: AppSettings.smallFontSize
		color: _control.enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
	}

	background: Rectangle {
		border.color: AppSettings.fontColor
		opacity: enabled ? 0.9 : 0.4
		color: _control.checked ? AppSettings.primaryDarkColor : AppSettings.primaryColor
		radius: 5
	}
}
