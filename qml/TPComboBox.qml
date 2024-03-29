import QtQuick
import QtQuick.Controls

ComboBox {
	id: control
	font.bold: true
	font.pixelSize: AppSettings.fontSizeText
	implicitWidth: fontMetrics.boundingRect("LorenIpsuM").width + 15
	implicitHeight: fontMetrics.boundingRect("LorenIpsuM").height + 20
	textRole: "text"
	valueRole: "value"

	property string textColor: "white"
	property string backgroundColor: "black"//"#c3cad5"

	FontMetrics {
		id: fontMetrics
		font.family: control.font.family
		font.pixelSize: AppSettings.fontSizeText
	}

	delegate: ItemDelegate {
		 width: control.width
		 contentItem: Text {
			text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
			color: textColor
			elide: Text.ElideRight
			font.pixelSize: AppSettings.fontSizeText
			font.weight: Font.ExtraBold
			verticalAlignment: Text.AlignVCenter
		}
		highlighted: control.highlightedIndex === index
	}

	indicator: Canvas {
		id: canvas
		x: control.width - width - control.rightPadding
		y: control.topPadding + (control.availableHeight - height) / 2
		width: 12
		height: 8
		contextType: "2d"

		Connections {
			target: control
			function onPressedChanged() { canvas.requestPaint(); }
		}

		onPaint: {
			if (context) {
				context.reset();
				context.moveTo(0, 0);
				context.lineTo(width, 0);
				context.lineTo(width / 2, height);
				context.closePath();
				context.fillStyle = textColor
				context.fill();
			}
		}
	}

	contentItem: Text {
		text: control.displayText
		leftPadding: 5
		rightPadding: control.indicator.width + control.spacing
		color: textColor
		font.pixelSize: AppSettings.fontSizeText
		font.weight: Font.ExtraBold
		verticalAlignment: Text.AlignVCenter
		elide: Text.ElideRight
	}

	background: Rectangle {
		implicitWidth: control.implicitWidth
		implicitHeight: control.implicitHeight
		border.color: "black"
		color: backgroundColor
		opacity: 0.5
		border.width: control.visualFocus ? 2 : 1
		radius: 6
	}

	popup: Popup {
		y: control.height - 1
		width: control.width
		implicitHeight: contentItem.implicitHeight
		padding: 1

		contentItem: ListView {
			implicitHeight: contentHeight
			model: control.popup.visible ? control.delegateModel : null
			currentIndex: control.highlightedIndex

			delegate: Text {
				id: delegate
				text: model.key
				color: control.highlighted ? primaryLightColor : "white"
				font.pixelSize: AppSettings.fontSizeText
				font.weight: Font.ExtraBold
				padding: 0
			}
			ScrollIndicator.vertical: ScrollIndicator { }
		}

		background: Rectangle {
			border.color: "black"
			color: backgroundColor
			opacity: 0.5
			radius: 6
		}
	}
}
