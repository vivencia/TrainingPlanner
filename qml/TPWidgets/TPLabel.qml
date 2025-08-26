import QtQuick
import QtQuick.Controls

import "../"

Label {
	id: control
	color: enabled ? fontColor : appSettings.disabledFontColor
	wrapMode: singleLine ? Text.NoWrap : Text.WordWrap
	elide: Text.ElideNone
	font: AppGlobals.regularFont
	minimumPixelSize: appSettings.smallFontSize * 0.7
	fontSizeMode: Text.Fit
	verticalAlignment: Text.AlignVCenter
	topInset: 0
	bottomInset: 0
	leftInset: 0
	rightInset: 0
	padding: 0
	property string fontColor: appSettings.fontColor
	property bool singleLine: true

	/*background: Rectangle {
		color: "transparent"
		border.color: "black"
	}*/
}
