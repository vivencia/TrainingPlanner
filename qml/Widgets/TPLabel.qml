import QtQuick
import QtQuick.Controls

import TpQml

Label {
	id: control
	color: enabled ? fontColor : AppSettings.disabledFontColor
	wrapMode: singleLine ? Text.NoWrap : Text.WordWrap
	font: AppGlobals.regularFont
	minimumPixelSize: AppSettings.smallFontSize * 0.5
	fontSizeMode: Text.Fit
	verticalAlignment: Text.AlignVCenter
	topInset: 0
	bottomInset: 0
	leftInset: 0
	rightInset: 0
	padding: 0

	property string fontColor: AppSettings.fontColor
	property bool singleLine: true
	property bool useBackground: false
	property string backgroundColor: AppSettings.primaryDarkColor

	background: useBackground ? itemBack : null

	Rectangle {
		id: itemBack
		color: control.enabled ? backgroundColor : "transparent"
		radius: 8
		opacity: 0.5
	}
}
