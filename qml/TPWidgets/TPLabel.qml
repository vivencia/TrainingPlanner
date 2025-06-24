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
	fontSizeMode: Text.FixedSize
	width: _preferredWidth
	height: _preferredHeight
	verticalAlignment: Text.AlignVCenter
	topInset: 0
	bottomInset: 0
	leftInset: 0
	rightInset: 0
	padding: 0
	clip: true

	property string fontColor: appSettings.fontColor
	property int widthAvailable: width
	property int heightAvailable: appSettings.itemDefaultHeight
	property bool singleLine: wrapMode === Text.NoWrap ? true : width > _textWidth
	property int lineCount: singleLine ? 1 : Math.ceil(_textWidth/width) + 1
	property int _textWidth
	property int _textHeight
	readonly property int _preferredWidth: widthAvailable >= 20 ? Math.min(_textWidth*1.5, widthAvailable) : _textWidth*1.5
	readonly property int _preferredHeight: singleLine ? heightAvailable : Math.max(lineCount * _textHeight * 1.5, heightAvailable);

	readonly property FontMetrics currentFontMetrics: FontMetrics {
		font.family: control.font.family
		font.pixelSize: control.font.pixelSize
		font.weight: control.font.weight
	}

	signal sizeChanged()

	function preferredLineCount(): int {
		return Math.ceil(_textWidth/width) + 1;
	}

	function preferredHeight(): int {
		return heightAvailable !== appSettings.itemDefaultHeight ?
						Math.min(preferredLineCount() * _textHeight, heightAvailable) : preferredLineCount() * _textHeight;
	}

	/*Rectangle {
		border.color: appSettings.fontColor
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
		_textWidth = currentFontMetrics.boundingRect(text).width*1.3;
		_textHeight = currentFontMetrics.boundingRect("TP").height*1.3;
		const hasNewLineEscapeChr = text.indexOf('\n') !== -1;
		if (hasNewLineEscapeChr)
			singleLine = false;
		lineCount = (singleLine ? 1 : Math.ceil(_textWidth/widthAvailable)) + (hasNewLineEscapeChr ? text.split('\n').length - 1: 0);
		if (_textWidth > control.width)
		{
			fontSizeMode = Text.Fit;
			//if (font.pixelSize > AppGlobals.smallFont.pixelSize)
			//	font.pixelSize *= 0.9;
			if (!singleLine) {
				if (control.height >= 15 && lineCount * font.pixelSize*1.35 > control.height)
					wrapMode = Text.WordWrap;
			}
			else
				elide = Text.ElideMiddle;
		}
		sizeChanged();
	}
}
