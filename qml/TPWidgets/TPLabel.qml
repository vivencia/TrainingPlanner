import QtQuick
import QtQuick.Controls

import "../"

Label {
	id: control
	color: enabled ? fontColor : appSettings.disabledFontColor
	wrapMode: Text.NoWrap
	elide: Text.ElideNone
	font: AppGlobals.regularFont
	minimumPointSize: appSettings.smallFontSize
	maximumLineCount: singleLine ? 1 : 50
	fontSizeMode: Text.Fit
	width: _preferredWidth
	height: _preferredHeight
	verticalAlignment: Text.AlignVCenter

	property string fontColor: appSettings.fontColor
	property int widthAvailable: appSettings.pageWidth - 20
	property int heightAvailable: 25
	property bool singleLine: wrapMode === Text.NoWrap ? true : width > _textWidth

	property int _textWidth
	property int _textHeight
	readonly property int _preferredWidth: Math.min(_textWidth, widthAvailable)
	readonly property int _lineCount: singleLine ? 1 : Math.ceil(_textWidth/widthAvailable) + 1
	readonly property int _preferredHeight: singleLine ? heightAvailable : heightAvailable != 25 ? Math.min(_lineCount * _textHeight, heightAvailable) : _lineCount * _textHeight;

	onTextChanged: text => {
		if (text.length > 0)
			adjustTextSize();
	}

	function adjustTextSize() {
		if (font === AppGlobals.regularFont) {
			_textWidth = AppGlobals.fontMetricsRegular.boundingRect(text).width
			_textHeight = AppGlobals.fontMetricsRegular.boundingRect("TP").height
		}
		else if (font === AppGlobals.smallFont) {
			_textWidth = AppGlobals.fontMetricsSmall.boundingRect(text).width
			_textHeight = AppGlobals.fontMetricsSmall.boundingRect("TP").height
		}
		else if (font === AppGlobals.largeFont) {
			_textWidth = AppGlobals.fontMetricsLarge.boundingRect(text).width
			_textHeight = AppGlobals.fontMetricsLarge.boundingRect("TP").height
		}
		else if (font === AppGlobals.extraLargeFont) {
			_textWidth = AppGlobals.fontMetricsExtraLarge.boundingRect(text).width
			_textHeight = AppGlobals.fontMetricsExtraLarge.boundingRect("TP").height
		}

		if (_textWidth > control.width)
		{
			if (font.pixelSize > AppGlobals.smallFont.pixelSize)
				font.pixelSize *= 0.9;
			if (lineCount * font.pixelSize*1.35 > control.height)
				wrapMode = Text.WordWrap;
		}
	}
}
