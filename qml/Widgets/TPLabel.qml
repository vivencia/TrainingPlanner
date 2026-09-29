import QtQuick
import QtQuick.Controls

import TpQml

Label {
	id: _control
	color: enabled ? fontColor : AppSettings.disabledFontColor
	wrapMode: singleLine ? Label.NoWrap : Label.WrapAtWordBoundaryOrAnywhere
	font: AppGlobals.regularFont
	minimumPixelSize: singleLine ? AppSettings.smallFontSize * 0.5 : AppSettings.smallFontSize
	fontSizeMode: singleLine ? Label.Fit : Label.VerticalFit
	verticalAlignment: Label.AlignVCenter
	horizontalAlignment: Label.AlignLeft
	background: useBackground ? itemBack : null
	topInset: 0
	bottomInset: 0
	leftInset: 0
	rightInset: 0
	padding: 0

	FontMetrics {
		id: fm
		font: _control.font
	}

	property string fontColor: AppSettings.fontColor
	property bool singleLine: true
	property bool useBackground: false
	property bool showBorder: false
	property string backgroundColor: AppSettings.primaryLightColor
	readonly property int preferredWidth: Math.min(fm.boundingRect(text).width, AppSettings.pageWidth * 0.9)

	Rectangle {
		id: itemBack
		color: _control.enabled ? _control.backgroundColor : Qt.lighter(_control.backgroundColor, 1.5)
		border.color: _control.showBorder ? AppSettings.fontColor : "transparent"
		radius: 8
		opacity: 0.7
	}

	function preferredHeight(): int {
		if (singleLine)
			return fm.height * 1.05;

		const br_w = fm.boundingRect(text).width;
		let ph = 0;
		if (width < br_w) {
			ph = Math.floor(fm.height * (fm.boundingRect(text).width / width));
			return ph;
		} else {
			return Math.max(fm.height, height);
		}
	}
}
