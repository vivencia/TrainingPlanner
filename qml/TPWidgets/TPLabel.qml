import QtQuick
import QtQuick.Controls

import "../"

Label {
	id: control
	color: enabled ? fontColor : appSettings.disabledFontColor
	wrapMode: singleLine ? Text.NoWrap : Text.WordWrap
	font: AppGlobals.regularFont
	minimumPixelSize: appSettings.smallFontSize * 0.9
	fontSizeMode: Text.Fit
	verticalAlignment: Text.AlignVCenter
	topInset: 0
	bottomInset: 0
	leftInset: 0
	rightInset: 0
	padding: 0

	property string fontColor: appSettings.fontColor
	property bool singleLine: true
	property bool useBackground: false

	background: useBackground ? itemBack : null

	Rectangle {
		id: itemBack
		color: control.enabled ? appSettings.primaryDarkColor : "transparent"
		radius: 6
		opacity: 0.5
	}

	/*background: Rectangle {
		color: "transparent"
		border.color: "black"
	}*/
}
