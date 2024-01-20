import QtQuick
import QtQuick.Controls

TextField {
	property string textColor: "white"
	property string backgroundColor: "#c3cad5"

	id: control
	font.pixelSize: AppSettings.fontSizeText
	font.weight: Font.ExtraBold
	color: textColor
	wrapMode: Text.WordWrap
	leftInset: 0
	rightInset: 0
	topInset: 0
	bottomInset: 0
	leftPadding: 5
	topPadding: 0
	bottomPadding: 0
	rightPadding: 5
	implicitWidth: fontMetrics.boundingRect("LorenIpsuM").width + 15
	implicitHeight: fontMetrics.boundingRect("LorenIpsuM").height + 10

	FontMetrics {
		id: fontMetrics
		font.family: control.font.family
		font.pixelSize: AppSettings.fontSizeText
	}

	background: Rectangle {
		id: itemBack
		border.color: "black"
		color: backgroundColor
		radius: 6
		opacity: 0.8
	}
}
