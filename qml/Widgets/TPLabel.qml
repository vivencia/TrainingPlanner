pragma ComponentBehavior: Bound

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
	height: visible ? preferredHeight() : 0

	property string fontColor: AppSettings.fontColor
	property bool singleLine: true
	property bool useBackground: false
	property bool showBorder: false
	property string helpString
	property int questionMarkOffset: 0
	property string backgroundColor: AppSettings.primaryLightColor
	readonly property int preferredWidth: Math.min(fm.boundingRect(text).width, AppSettings.pageWidth * 0.9)

	Rectangle {
		id: itemBack
		color: _control.enabled ? _control.backgroundColor : Qt.lighter(_control.backgroundColor, 1.5)
		border.color: _control.showBorder ? AppSettings.fontColor : "transparent"
		radius: 8
		opacity: 0.7
	}

	Loader {
		id: helpLoader
		asynchronous: true
		active: _control.helpString.length > 0

		anchors {
			verticalCenter: parent.verticalCenter
			right: parent.right
			rightMargin: -_control.questionMarkOffset
		}

		sourceComponent: TPButton {
			imageSource: "question.png"
			width: AppSettings.itemDefaultHeight
			height: width

			onClicked: helpTip.show(_control.helpString, 5000);

			ToolTip {
				id: helpTip
				closePolicy: Popup.CloseOnPressOutside|Popup.CloseOnEscape
				z: 2
			}
		}
	}

	FontMetrics {
		id: fm
		font: _control.font
	}

	function preferredHeight(): int {
		let ph = 0;
		if (singleLine) {
			ph = fm.height * 1.05;
		} else {
			let br = 1, br1 = 0, n_lines = 0;
			do {
				++n_lines;
				br1 = text.indexOf("<br>", br);
				//if ((br1 - br) * fm.maximumCharacterWidth >= width)
				//	++n_lines;
				br = br1 + 1;
			} while (br > 1);
			if (n_lines === 1) {
				const br_w = fm.boundingRect(text).width;
				if (width < br_w)
					ph = Math.floor(fm.height * (fm.boundingRect(text).width / width));
				else
					ph = fm.height * 1.05;
			} else {
				ph = n_lines * fm.height * 1.05;
			}
		}
		return ph;
	}
}
