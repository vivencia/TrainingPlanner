import QtQuick
import QtQuick.Controls

import "../"

Label {
	id: control
	color: enabled ? enabledFontColor : "darkgrey"
	wrapMode: Text.WordWrap
	font: AppGlobals.textFont
	minimumPointSize: appSettings.fontSizeLists
	maximumLineCount: singleLine ? 1 : 50
	fontSizeMode: elide !== Text.ElideNone ? Text.FixedSize : Text.Fit
	topPadding: lineCount > 1 ? -2*lineCount : 0
	width: preferredWidth
	height: preferredHeight

	property bool singleLine: elide !== Text.ElideNone ? true : width > textWidth
	property string enabledFontColor: appSettings.fontColor
	readonly property int textWidth: AppGlobals.fontMetricsText.boundingRect(text).width
	readonly property int textHeight: AppGlobals.fontMetricsText.boundingRect(text).height
	readonly property int preferredWidth: textWidth
	readonly property int preferredHeight: singleLine ? 25 : 2*(Math.ceil(textWidth/(appSettings.pageWidth-40)) * textHeight)
	readonly property int lineCount: singleLine ? 1 : Math.ceil(textWidth / width) + 1
}
