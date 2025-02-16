import QtQuick
import QtQuick.Controls

import "../"

Label {
	id: control
	color: enabled ? fontColor : appSettings.disabledFontColor
	wrapMode: Text.NoWrap
	elide: Text.ElideNone
	font: AppGlobals.regularFont
	minimumPixelSize: appSettings.smallFontSize
	maximumLineCount: singleLine ? 1 : 50
	fontSizeMode: Text.Fit
	width: _preferredWidth
	height: _preferredHeight
	verticalAlignment: Text.AlignVCenter

	property string fontColor: appSettings.fontColor
	property int widthAvailable: appSettings.pageWidth - 20
	property int heightAvailable: 25
	property bool singleLine: wrapMode === Text.NoWrap ? true : width > _textWidth
	property int lineCount: singleLine ? 1 : Math.ceil(_textWidth/width) + 1

	property int _textWidth
	property int _textHeight
	readonly property int _preferredWidth: widthAvailable >= 20 ? Math.min(_textWidth, widthAvailable) : _textWidth
	readonly property int _preferredHeight: singleLine ? heightAvailable : heightAvailable != 25 ? Math.min(lineCount * _textHeight, heightAvailable) : lineCount * _textHeight;

	readonly property FontMetrics currentFontMetrics: FontMetrics {
		font.family: control.font.family
		font.pixelSize: control.font.pixelSize
		font.weight: control.font.weight
	}

	signal sizeChanged()

	/*Rectangle {
		border.color: "black"
		color: "transparent"
		anchors.fill: parent
	}*/

	onTextChanged: text => {
		if (text.length > 0)
			adjustTextSize();		
	}

	Component.onCompleted: {
		appSettings.fontSizeChanged.connect(adjustTextSize);
		appTr.applicationLanguageChanged.connect(adjustTextSize);
	}

	function adjustTextSize() {
		if (text.length === 0) return;
		_textWidth = currentFontMetrics.boundingRect(text).width
		_textHeight = currentFontMetrics.boundingRect("TP").height
		const hasNewLineEscapeChr = text.indexOf('\n') !== -1;
		singleLine = hasNewLineEscapeChr ? false : width > _textWidth;
		lineCount = (singleLine ? 0 : Math.ceil(_textWidth/widthAvailable)) + (hasNewLineEscapeChr ? text.split('\n').length - 1: 0);
		if (_textWidth > control.width)
		{
			if (font.pixelSize > AppGlobals.smallFont.pixelSize)
				font.pixelSize *= 0.9;
			if (!singleLine) {
				if (control.height >= 15 && lineCount * font.pixelSize*1.35 > control.height)
					wrapMode = Text.WordWrap;
			}
		}
		sizeChanged();
	}
}
