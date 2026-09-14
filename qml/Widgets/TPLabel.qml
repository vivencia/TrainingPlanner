import QtQuick
import QtQuick.Controls

import TpQml

Label {
	id: _control
	color: enabled ? fontColor : AppSettings.disabledFontColor
	wrapMode: singleLine ? Label.NoWrap : Label.WrapAtWordBoundaryOrAnywhere
	font: AppGlobals.regularFont
	minimumPixelSize: AppSettings.smallFontSize * 0.5
	fontSizeMode: singleLine ? Label.Fit : Label.VerticalFit
	verticalAlignment: Label.AlignVCenter
	horizontalAlignment: Label.AlignLeft
	background: useBackground ? itemBack : null
	topInset: 0
	bottomInset: 0
	leftInset: 0
	rightInset: 0
	padding: 0

	property string fontColor: AppSettings.fontColor
	property bool singleLine: true
	property bool useBackground: false
	property string backgroundColor: AppSettings.primaryDarkColor

	Rectangle {
		id: itemBack
		color: _control.enabled ? _control.backgroundColor : "transparent"
		radius: 8
		opacity: 0.7
	}
}
