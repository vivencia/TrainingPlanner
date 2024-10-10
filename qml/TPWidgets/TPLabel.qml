import QtQuick
import QtQuick.Controls

import "../"

Label {
	id: control
	color: enabled ? fontColor : appSettings.disabledFontColor
	wrapMode: Text.WordWrap
	font: AppGlobals.textFont
	minimumPointSize: appSettings.fontSizeLists
	maximumLineCount: singleLine ? 1 : 50
	fontSizeMode: elide !== Text.ElideNone ? Text.FixedSize : Text.Fit
	topPadding: _lineCount > 1 ? -2*_lineCount : 2
	width: _preferredWidth
	height: _preferredHeight

	property string fontColor: appSettings.fontColor
	property int widthAvailable: appSettings.pageWidth - 20
	property int heightAvailable: 25
	property bool singleLine: elide !== Text.ElideNone ? true : widthAvailable > _textWidth

	readonly property int _textWidth: AppGlobals.fontMetricsText.boundingRect(text).width
	readonly property int _textHeight: AppGlobals.fontMetricsText.boundingRect("TP").height
	readonly property int _preferredWidth: Math.min(_textWidth, widthAvailable)
	readonly property int _lineCount: singleLine ? 1 : Math.ceil(_textWidth/widthAvailable) + 1
	readonly property int _preferredHeight: singleLine ? heightAvailable : heightAvailable != 25 ? Math.min(_lineCount * _textHeight, heightAvailable) : _lineCount * _textHeight;

	/*property FontMetrics _fontMetrics: AppGlobals.fontMetricsText

	onFontChanged: {
		if (control.font.pointSize === appSettings.fontSizeTitle)
			_fontMetrics = AppGlobals.fontMetricsTitle;
	}*/
}
